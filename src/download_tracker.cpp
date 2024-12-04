#include <string.h>
#include <stdio.h>
#include "print_utils.h"
#include "download_tracker.h"

constexpr uint32_t PAYLOAD_START = 10;
constexpr uint32_t BLOCK_BYTES = 51;
constexpr uint32_t RX_BLOCKS = 1;
constexpr uint32_t RX_BYTES = BLOCK_BYTES * RX_BLOCKS;

constexpr uint32_t NUM_SEGMENTS = 16;
constexpr uint32_t RX_SEQ_LEN = RX_BYTES * NUM_SEGMENTS; // Bytes


DownloadTracker::DownloadTracker(file_t f, const char* dest_path, RxQueue* rxq, request_block_t cb):
    _f(f),
    _rxq(rxq),
    _cb(cb),
    _stop(false),
    _is_stopped(true),
    _is_file_ready(false),
    _block_count(0),
    _bytes_written(0),
    _bytes_received(0)
{
    _current_block = f.start_block;
    _reset_segments();
}

void DownloadTracker::_reset_segments() {
    _chunk_size = 0;
    //TODO: reset_segments;
}

void DownloadTracker::start() {
    _thdl = std::thread(&DownloadTracker::_run_download, this);
    _thwr = std::thread(&DownloadTracker::_run_write, this);
}

void DownloadTracker::_run_download() {
    printf("Download thread started\n");
}

void DownloadTracker::_run_write() {
    printf("Download thread started\n");
}

void DownloadTracker::_flush_rx_queue() {
    while( !_rxq->is_empty() ) {
        _rxq->get();
    }
}
