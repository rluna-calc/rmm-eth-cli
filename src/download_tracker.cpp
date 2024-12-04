#include <string.h>
#include <stdio.h>
#include <chrono>
#include "print_utils.h"
#include "time_utils.h"
#include "download_tracker.h"

constexpr uint32_t PAYLOAD_START = 10;
constexpr uint32_t BLOCK_BYTES = 51;
constexpr uint32_t RX_BLOCKS = 1;
constexpr uint32_t RX_BYTES = BLOCK_BYTES * RX_BLOCKS;

constexpr uint32_t NUM_SEGMENTS = 16;
constexpr uint32_t RX_SEQ_LEN = RX_BYTES * NUM_SEGMENTS; // Bytes

constexpr int32_t MAX_NUM_ACTIVE_REQUESTS = 1;

DownloadTracker::DownloadTracker(file_t f, const char* dest_path, RxQueue* rxq, callback_t cb):
    _f(f),
    _rxq(rxq),
    _cb_request_block(cb),
    _stop(false),
    _is_stopped(true),
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
    _th_process = std::thread(&DownloadTracker::_run_process, this);
    _wait_for_process_ready();

    _th_write = std::thread(&DownloadTracker::_run_write, this);
    _wait_for_file_ready();

    _th_request = std::thread(&DownloadTracker::_run_request, this);
}

void DownloadTracker::_run_request() {
    printf("Request thread started\n");

    _reset_segments();
    _flush_rx_queue();

    while( !_stop && !_stop_requesting) {

        // TODO: maybe use a CV to avoid a spin loop?
        if( _num_active_requests < MAX_NUM_ACTIVE_REQUESTS ) {
            _cb_request_block(_current_block); // move to download thread
            _num_active_requests++;
        }
    }
}

void DownloadTracker::_run_process() {
    printf("Process thread started\n");

    _is_stopped = false;
    bool is_finished = false;
    int32_t num_retries = 0;
    uint64_t bytes_prev = 0;
    int64_t time_prev = 0;
    bool ok_block_increment = false;

    int64_t start_time = time_since_epoch_seconds();

    _is_process_ready = true;
    while( !_stop && !is_finished) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
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
