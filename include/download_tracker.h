#ifndef _DOWNLOAD_TRACKER_H_
#define _DOWNLOAD_TRACKER_H_

#include <vector>
#include <mutex>
#include <stdint.h>
#include <stdbool.h>
#include <string>
#include <thread>
#include <rx_queue.h>
#include <functional>

typedef struct {
    std::string filename;
    uint64_t start_block;
    uint64_t block_count;
    uint64_t file_size;
    char created[17];
} file_t;


typedef void (*request_block_t)(void* puser);

struct DownloadTracker {
    using callback_t = std::function<void(uint64_t block_num)>;

    DownloadTracker(file_t f, const char* dest_path, RxQueue* rxq, callback_t cb);
    ~DownloadTracker() {};

    void start();
    void stop() { _stop = true; }

    void _reset_segments();
    void _flush_rx_queue();

    void _run_request();
    void _run_process();
    void _run_write();
    void _wait_for_file_ready();
    void _wait_for_process_ready();
    bool _is_stopeed() { return _is_stopped; }

    file_t _f;
    RxQueue* _rxq;
    callback_t _cb_request_block;
    std::thread _th_request;
    std::thread _th_process;
    std::thread _th_write;

    bool _stop;
    bool _is_stopped;
    bool _is_file_ready;
    bool _is_process_ready;
    bool _stop_requesting;
    uint64_t _block_count;
    uint64_t _bytes_written;
    uint64_t _bytes_received;
    uint64_t _current_block;
    uint64_t _chunk_size;
    int32_t _num_active_requests;
};

#endif
