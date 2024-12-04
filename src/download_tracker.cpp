#include <string.h>
#include <stdio.h>
#include <chrono>
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
    _th_write = std::thread(&DownloadTracker::_run_write, this);
    _wait_for_file_ready();

    _th_process = std::thread(&DownloadTracker::_run_process, this);
    _wait_for_process_ready();

    _th_request = std::thread(&DownloadTracker::_run_request, this);
}

bool DownloadTracker::get_is_stopped() {
    return (!_is_file_ready && !_is_process_ready);
}

void DownloadTracker::_run_request() {
    printf("Request thread started\n");

    _reset_segments();
    _flush_rx_queue();

    while( !(_stop || _stop_requesting) ) {

        // TODO: maybe use a CV to avoid a spin loop?
        if( _num_active_requests < MAX_NUM_ACTIVE_REQUESTS ) {
            // printf("%d: current_block request = %llu\n", __LINE__, _current_block);
            _cb_request_block(_current_block);
            _num_active_requests++;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }

    printf("Request thread finished\n");
}

void DownloadTracker::_run_process() {
    printf("Process thread started\n");

    int32_t num_retries = 0;
    uint64_t bytes_prev = 0;
    int64_t time_prev = 0;
    bool ok_block_increment = false;

    int64_t start_time = time_since_epoch_seconds();

    _is_process_ready = true;
    q_elem_t* elem;
    printf("dlt _rxq = %p\n", _rxq);
    while( !(_stop || _stop_requesting) ) {
        _reset_segments();
        for(int i = 0; i < NUM_SEGMENTS; i++) {    
            elem = _rxq->get_with_timeout_ms(200);
            if(elem) {
                ok_block_increment = _process_segment(elem);
                printf("%llu: %d: num_elems = %d, chunk_size = %d, ok_inc = %d\n", time_since_eqoch_microsecs(), __LINE__, 
                    _rxq->_num_elems, _chunk_size, ok_block_increment);

                if (ok_block_increment) {
                    break;
                }
            }
            else {
                printf("RX queue empty on segment %d\n", i);
                break;
                // exit(1);
            }
        }

        _stop_requesting = _check_segment(ok_block_increment); // break; if true
        
        if (_block_count % REPORT_COUNT == 0) {
            _do_reporting();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    }

    _is_process_ready = false;
    printf("Process thread finished\n");
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

void DownloadTracker::_do_reporting() {

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

    while( !_stop ) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    _is_file_ready = false;
    printf("Write thread finished\n");
}

void DownloadTracker::_flush_rx_queue() {
    while( !_rxq->is_empty() ) {
        _rxq->get();
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
