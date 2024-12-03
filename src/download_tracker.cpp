#include <string.h>
#include <stdio.h>
#include "print_utils.h"
#include "download_tracker.h"

DownloadTracker::DownloadTracker(file_t* f, const char* dest_path, RxQueue* rxq, request_block_t cb):
    _rxq(rxq),
    _cb(cb)
{
    
}
