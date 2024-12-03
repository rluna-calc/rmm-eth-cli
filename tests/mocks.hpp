#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "udp_utils.h"
#include "rx_queue.h"

// Mock for RxQueue
struct MockRxQueue {
    MOCK_METHOD(void, push, (q_elem_t elem), ());
    MOCK_METHOD(bool, pop, (q_elem_t* elem), ());
};

// Mock for the Receiver class
struct MockUdpTxRx {
    MockUdpTxRx(int port, RxQueue* rxq) {}

    // Mock methods
    // MOCK_METHOD(void, start, (), ());
    // MOCK_METHOD(void, stop, (), ());
    void start() {};
    void stop() {}

    // Mocked constructor and destructor if necessary
    MOCK_METHOD(void, set_port, (), ());
    MOCK_METHOD(void, set_socket, (), ());
    MOCK_METHOD(void, set_stop, (), ());
    MOCK_METHOD(void, set_is_running, (), ());

    // Mock for the send_udp_packet function
    MOCK_METHOD(void, send_udp_packet, (const char* ip, int port, const uint8_t* buffer, const int len), ());

};

 