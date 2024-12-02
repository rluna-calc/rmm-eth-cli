#ifndef _UDP_UTILS_H_
#define _UDP_UTILS_H_

#include <stdint.h>
#include <stdbool.h>
#include <thread>
#include "rx_queue.h"

constexpr uint32_t NUM_BUFFER_POOL = 16;
constexpr uint32_t BUFFER_SIZE = 9000;

struct Receiver {
    Receiver(int port, RxQueue* rxq);
    ~Receiver();

    void start();
    void stop() { _stop = true; }
    bool is_running() { return _is_running; }

    void _run();
    void _close_socket();

    int _port;
    int _sock;
    bool _stop;
    bool _is_running;
    RxQueue* _q;
    std::thread _th;

    uint8_t _buffer_pool[NUM_BUFFER_POOL][BUFFER_SIZE];
    uint32_t _buf_idx;
};

void send_udp_packet(const char* ip, int port, const uint8_t* buffer, const int len);

#endif