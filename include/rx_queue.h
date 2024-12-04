#ifndef _RX_QUEUE_H_
#define _RX_QUEUE_H_

#include <vector>
#include <mutex>
#include <stdint.h>
#include <stdbool.h>

constexpr uint32_t RX_BUFFER_SIZE = 9000;

typedef struct {
    int32_t len;
    uint8_t buf[RX_BUFFER_SIZE];
} q_elem_t;

struct RxQueue {
    RxQueue(int size);

    bool push(q_elem_t* new_elem);
    q_elem_t* get();
    bool is_empty() { return (_num_elems == 0); }

    uint32_t _wrap(uint32_t);

    std::vector<q_elem_t> _buf;
    uint32_t _head;
    uint32_t _tail;
    uint32_t _num_elems;
    uint32_t _max_elems;

    std::mutex _mutex;
};

#endif