#ifndef _RX_QUEUE_H_
#define _RX_QUEUE_H_

#include <vector>
#include <mutex>
#include <stdint.h>
#include <stdbool.h>


struct RxQueue {
    RxQueue(int size);

    bool push(uint8_t* new_buf);
    uint8_t* get();

    uint32_t _wrap(uint32_t);

    std::vector<uint8_t*> _buf;
    uint32_t _head;
    uint32_t _tail;
    uint32_t _num_elems;
    uint32_t _max_elems;

    std::mutex _mutex;
};

#endif