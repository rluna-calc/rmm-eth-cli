#ifndef _DOWNLOAD_TRACKER_H_
#define _DOWNLOAD_TRACKER_H_

#include <vector>
#include <mutex>
#include <stdint.h>
#include <stdbool.h>
#include <string>
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
    DownloadTracker(file_t* f, const char* dest_path, RxQueue* rxq, request_block_t cb);
    ~DownloadTracker();

    void init();

    RxQueue* _rxq;
    request_block_t _cb;
};

#endif