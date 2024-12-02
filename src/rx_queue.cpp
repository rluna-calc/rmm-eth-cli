#include "rx_queue.h"


RxQueue::RxQueue(int size) {
    for (int i = 0; i < size; i++) {
        _buf.push_back(nullptr);
    }

    _max_elems = size;
    _head = 0;
    _tail = 0;
    _num_elems = 0;
}

bool RxQueue::push(uint8_t* new_buf) {
    std::lock_guard<std::mutex> lock(_mutex);

    bool ret = false;
    if (_num_elems < _max_elems) {
        _buf[_head++] = new_buf;
        _num_elems++;
        _head = _wrap(_head);

        ret = true;
    }

    return ret;
}

uint8_t* RxQueue::get() {
    std::lock_guard<std::mutex> lock(_mutex);

    uint8_t* ret = nullptr;
    if (_num_elems > 0) {
        ret = _buf[_tail++];
        _num_elems--;
        _tail = _wrap(_tail);
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
