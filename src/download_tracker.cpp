#include <string.h>
#include <stdio.h>
#include <chrono>
#include <sstream>
#include <iomanip>
#include "print_utils.h"
#include "time_utils.h"
#include "download_tracker.h"

constexpr uint32_t REPORT_COUNT = 1000;

constexpr uint32_t PAYLOAD_START = 10;
constexpr uint32_t BLOCK_BYTES = 512;
constexpr uint32_t RX_BLOCKS = 16;
constexpr uint32_t RX_BYTES = BLOCK_BYTES * RX_BLOCKS;

constexpr uint32_t NUM_SEGMENTS = 16;
constexpr uint32_t RX_SEQ_LEN = RX_BYTES * NUM_SEGMENTS; // Bytes

constexpr int32_t MAX_NUM_ACTIVE_REQUESTS = 1;

DownloadTracker::DownloadTracker(file_t f, const char* dest_path, RxQueue* rxq, callback_t cb):
    _f(f),
    _rxq(rxq),
    _cb_request_block(cb),
    _stop(false),
    _is_file_ready(false),
    _is_process_ready(false),
    _stop_requesting(false),
    _block_count(0),
    _bytes_written(0),
    _bytes_received(0),
    _num_active_requests(0)
{
    _current_block = f.start_block;
    _reset_segments();
}

void DownloadTracker::_reset_segments() {
    _chunk_size = 0;
    //TODO: reset_segments;
}

void DownloadTracker::start() {
    _stop = false;
    _th_write = std::thread(&DownloadTracker::_run_write, this);
    _th_process = std::thread(&DownloadTracker::_run_process, this);
    _th_request = std::thread(&DownloadTracker::_run_request, this);
}

bool DownloadTracker::get_is_stopped() {
    return (!_is_file_ready && !_is_process_ready);
}

void DownloadTracker::wait_for_ready() {
    _wait_for_file_ready();
    _wait_for_process_ready();
}

void DownloadTracker::_run_request() {
    printf("Request thread started\n");

    _wait_for_file_ready();
    _wait_for_process_ready();

    _reset_segments();
    _flush_rx_queue();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    while( !(_stop || _stop_requesting) ) {

        // TODO: maybe use a CV to avoid a spin loop?
        if( _num_active_requests < MAX_NUM_ACTIVE_REQUESTS ) {
            _cb_request_block(_current_block);
            printf("%lu: %d: Requested block %lu\n", time_since_epoch_microsecs(), __LINE__, _current_block);

            _num_active_requests++;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }

    printf("Request thread finished, stop_requesting = %d, stop = %d\n", _stop_requesting, _stop);
}

void DownloadTracker::_run_process() {
    printf("Process thread started\n");

    int64_t prev_bytes = 0;
    int64_t prev_time = 0;
    bool ok_block_increment = false;

    int64_t start_time = time_since_epoch_seconds();

    _is_process_ready = true;
    q_elem_t* elem;
    printf("dlt _rxq = %p\n", _rxq);
    uint32_t segment_counter = 0;

    _stop = false;
    while( !(_stop || _stop_requesting) ) {
        _reset_segments();
        segment_counter = 0;
        
        while ((segment_counter < NUM_SEGMENTS) && !_stop) {
            elem = _rxq->get(200);
            if(elem) {
                segment_counter++;
                ok_block_increment = _process_segment(elem);
                // printf("%llu: %d: num_elems = %d, chunk_size = %d, ok_inc = %d\n", time_since_epoch_microsecs(), __LINE__, 
                //     _rxq->_num_elems, _chunk_size, ok_block_increment);

                if (ok_block_increment) {
                    break;
                }
            }
            else {
                printf("RX queue empty on segment %d\n", segment_counter);
                // break;
                // exit(1);
            }
        }

        _stop_requesting = _check_segment(ok_block_increment); // break; if true
        
        if (_block_count % REPORT_COUNT == 0) {
            // _stop_requesting = _do_reporting(&prev_time, &prev_bytes);
            _do_reporting(&prev_time, &prev_bytes);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    _is_process_ready = false;
    printf("Process thread finished, stop_requesting = %d, stop = %d\n", _stop_requesting, _stop);
}

bool DownloadTracker::_check_segment(bool do_increment) {
    bool is_finished = false;

    if( do_increment ) {
        // Check for termination criteria
        if( _bytes_received >= _f.file_size ) {
            is_finished = true;
            _stop_requesting = true;
        }
        else { 
            _increment_block();
            _num_active_requests--; //TODO: move to write thread
        }
    }

    return is_finished;
}

bool DownloadTracker::_do_reporting(int64_t* prev_time, int64_t* prev_bytes) {
    int64_t time_now = time_since_epoch_microsecs();

    int64_t bytes_now = _block_count * BLOCK_BYTES;
    float rate = (float)(bytes_now - *prev_bytes) / (float)(time_now - *prev_time);
    printf("bytes [%lu, %lu], time [%lu, %lu]\n", bytes_now, *prev_bytes, time_now, *prev_time);

    bool end_now = true;
    int64_t bytes_remaining = _f.file_size - bytes_now;

    float time_remaining = -1.0;
    std::string remaining_str = "infinity";
    if (rate > 0) {
        time_remaining = bytes_remaining / rate;
        _get_time_str(time_remaining, remaining_str);
        end_now = false;
    }

    if (*prev_time > 0) {
        printf("%.1f GB of %.1f GB at ==> ", (float)bytes_now/1.e9, (float)_f.file_size/1.e9);
        printf("%.2f MB/s | %s\n", rate/1.e6, remaining_str.c_str());
    }

    *prev_time = time_now;
    *prev_bytes = bytes_now;

    return end_now;
}

void DownloadTracker::_get_time_str(float seconds, std::string& time_str) {

    std::ostringstream stream;
    if( seconds >= 3600. ) {
        float hours = seconds / 3600.;
        stream << std::fixed << std::setprecision(2) << hours << " hrs";
        time_str = stream.str();
    }
    else if( seconds >= 60. ) {
        float minutes = seconds / 60.;
        stream << std::fixed << std::setprecision(1) << minutes << " mins";
        time_str = stream.str();
    }
    else {
        stream << std::fixed << std::setprecision(1) << seconds << " secs";
        time_str = stream.str();
    }
}


void DownloadTracker::_increment_block() {
    uint64_t increment_value = (1 << 8);
    _block_count += increment_value;
    _current_block += increment_value;
}

bool DownloadTracker::_process_segment(q_elem_t* elem) {
    uint32_t reported_len = (elem->buf[3] << 8 | elem->buf[2]) * 2;
    if (elem->len - PAYLOAD_START == reported_len) {
        _bytes_received += reported_len;
        _chunk_size += reported_len;
        // _rx_segs.append(seg)
    }

    bool ok_to_increment = (_chunk_size == RX_SEQ_LEN);

    if (ok_to_increment) {
        // push to file_write queue to write to disk
        // if self.file_write_queue.full():
        //     LOG.warning("File write queue is full")
        // else:
        //     self.file_write_queue.put(self.rx_segs)
    }

    return ok_to_increment;
}

void DownloadTracker::_run_write() {
    printf("Write thread started\n");
    // created_microsecs = self.file_desc.created[-6:]
    // created_secs = self.file_desc.created[:-6]
    // filename = f"{self.dest_path}/"
    // filename += f"{self.file_desc.name}_{created_secs}_{created_microsecs}.ch10"
    // LOG.info(f"Ready to write file: {filename}")

    // open file
    _is_file_ready = true;

    _stop = false;
    while( !_stop ) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    _is_file_ready = false;
    printf("Write thread finished, _stop = %d\n", _stop);
}

void DownloadTracker::_flush_rx_queue() {
    while( !_rxq->is_empty() ) {
        _rxq->get(0);
    }
}

void DownloadTracker::_wait_for_file_ready() {
    //TODO: add a timeout

    while (!_is_file_ready) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void DownloadTracker::_wait_for_process_ready() {
    //TODO: add a timeout

    while (!_is_process_ready) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
