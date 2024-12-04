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
    }

    return ret;
}

q_elem_t* RxQueue::get() {
    std::lock_guard<std::mutex> lock(_mutex);

    q_elem_t* ret = nullptr;
    if (_num_elems > 0) {
        ret = &_buf[_tail++];
        _num_elems--;
        _tail = _wrap(_tail);
    }

    return ret;
}

q_elem_t* RxQueue::get_with_timeout_ms(uint32_t timeout_ms) {

    uint64_t start_time = time_since_epoch_microsecs();
    uint64_t end_time = timeout_ms * 1000;
    
    uint64_t time_now = 0;
    q_elem_t* elem = nullptr;
    while (time_now < end_time) {
        elem = get();        
        if(elem) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        time_now = time_since_epoch_microsecs();
    }

    return elem;
}

uint32_t RxQueue::_wrap(uint32_t val)
{
    if (val >= _max_elems) {
        val = 0;
    }

    return val;
}
