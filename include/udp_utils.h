#ifndef _UDP_UTILS_H_
#define _UDP_UTILS_H_

#include <stdint.h>
#include <stdbool.h>
#include <thread>
#include "rx_queue.h"


struct UdpTxRx {
    UdpTxRx(int port, RxQueue* rxq);
    ~UdpTxRx();

    void start();
    void stop() { _stop = true; }
    bool is_running() { return _is_running; }

    void _run();
    void _close_socket();

    static void send_udp_packet(const char* ip, int port, const uint8_t* buffer, const int len);

    int _port;
    int _sock;
    bool _stop;
    bool _is_running;
    RxQueue* _q;
    std::thread _th;

    q_elem_t _elem;
};


#endif