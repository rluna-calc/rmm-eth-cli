#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "udp_utils.h"
#include "rx_queue.h"

// Mock for RxQueue
class MockRxQueue : public RxQueue {
public:
    MOCK_METHOD(void, push, (q_elem_t elem), (override));
    MOCK_METHOD(bool, pop, (q_elem_t* elem), (override));
};

// Mock for the Receiver class
class MockReceiver : public Receiver {
public:
    MockReceiver(int port, RxQueue* rxq) : Receiver(port, rxq) {}

    // Mock methods
    MOCK_METHOD(void, start, (), (override));
    MOCK_METHOD(void, stop, (), (override));
    MOCK_METHOD(void, _run, (), (override));
    MOCK_METHOD(void, _close_socket, (), (override));

    // Mocked constructor and destructor if necessary
    MOCK_METHOD(void, set_port, (), ());
    MOCK_METHOD(void, set_socket, (), ());
    MOCK_METHOD(void, set_stop, (), ());
    MOCK_METHOD(void, set_is_running, (), ());
};

// Mock for the send_udp_packet function
MOCK_FUNCTION(void, send_udp_packet, (const char* ip, int port, const uint8_t* buffer, const int len), ());
 