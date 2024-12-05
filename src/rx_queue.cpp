#include "rx_queue.h"
#include <stdio.h>
#include <string.h>
#include <time_utils.h>
#include <chrono>
#include <thread>

RxQueue::RxQueue(int size) {
    q_elem_t elem = {0};
    for (int i = 0; i < size; i++) {
        _buf.push_back(elem);
    }

    _max_elems = size;
    _head = 0;
    _tail = 0;
    _num_elems = 0;
}

bool RxQueue::push(q_elem_t* new_elem) {
    std::lock_guard<std::mutex> lock(_mutex);

    bool ret = false;
    if (_num_elems < _max_elems) {
        memcpy(_buf[_head].buf, new_elem->buf, new_elem->len);
        _buf[_head].len = new_elem->len;
        _num_elems++;
        _head = _wrap(_head+1);
        ret = true;
        _c.notify_one();
    }

    return ret;
}

q_elem_t* RxQueue::get(uint32_t timeout) {
    chrono::milliseconds my_timeout = chrono::milliseconds(timeout);
    std::unique_lock<std::mutex> lock(_mutex);

    q_elem_t* ret = nullptr;

    // Wait until the queue is not empty or timeout occurs
    if (_c.wait_for(lock, my_timeout, [this] { return !is_empty(); })) {
        if (_num_elems > 0) {
            ret = &_buf[_tail++];
            _num_elems--;
            _tail = _wrap(_tail);
        }
    }

    return ret;
}

uint32_t RxQueue::_wrap(uint32_t val)
{
    if (val >= _max_elems) {
        val = 0;
    }

    return val;
}
