#ifndef _DOWNLOAD_TRACKER_H_
#define _DOWNLOAD_TRACKER_H_

#include <vector>
#include <mutex>
#include <stdint.h>
#include <stdbool.h>
#include <string>
#include <thread>
#include <rx_queue.h>

typedef struct {
    std::string filename;
    uint64_t start_block;
    uint64_t block_count;
    uint64_t file_size;
    char created[17];
} file_t;


typedef void (*request_block_t)(void);

struct DownloadTracker {
    DownloadTracker(file_t f, const char* dest_path, RxQueue* rxq, request_block_t cb);
    ~DownloadTracker();

    void start();
    void stop() { _stop = true; }

    void _reset_segments();
    void _flush_rx_queue();
    
    void _run_download();
    void _run_write();

    file_t _f;
    RxQueue* _rxq;
    request_block_t _cb;
    std::thread _thdl;
    std::thread _thwr;

    bool _stop;
    bool _is_stopped;
    bool _is_file_ready;
    uint64_t _block_count;
    uint64_t _bytes_written;
    uint64_t _bytes_received;
    uint64_t _current_block;
    uint64_t _chunk_size;
};

#endif