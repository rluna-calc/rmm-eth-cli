#include <gtest/gtest.h>
#include <stdint.h>
#include "rmm.h"
#include "mocks.hpp"

using rmm_t = Rmm<MockUdpTxRx>;

constexpr uint8_t DISCOVERY_RESP[] = {
    0x00,0x06,0x00,0x01,0x00,0x00,0x08,0x08,0x16,0x00,0x40,0x00,0xff,0x3f,0x37,0xc8,0x10,0x00,0x00,0x00,0x00,0x00,0x3f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x53,0x32,0x35,0x31,0x4e,0x58,0x41,0x48,0x33,0x34,0x39,0x37,0x30,0x32,0x58,0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x45,0x58,0x4d,0x30,0x32,0x42,0x36,0x51,0x53,0x61,0x6d,0x73,0x75,0x6e,0x67,0x20,0x53,0x53,0x44,0x20,0x38,0x35,0x30,0x20,0x50,0x52,0x4f,0x20,0x32,0x35,0x36,0x47,0x42,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x01,
    0x80,0x01,0x40,0x00,0x2f,0x00,0x40,0x00,0x02,0x00,0x02,0x07,0x00,0xff,0x3f,
    0x10,0x00,0x3f,0x00,0x10,0xfc,0xfb,0x00,0x01,0x01,0xff,0xff,0xff,0x0f,0x00,
    0x00,0x07,0x00,0x03,0x00,0x78,0x00,0x78,0x00,0x78,0x00,0x78,0x00,0x10,0x0f,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1f,0x00,0x0e,0x85,0xc6,
    0x00,0x6c,0x01,0x60,0x00,0xfc,0x03,0x39,0x00,0x6b,0x74,0x01,0x7d,0x63,0x41,
    0x69,0x74,0x01,0xbc,0x63,0x41,0x7f,0x40,0x01,0x00,0x01,0x00,0x00,0x00,0xfe,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0xc0,0xca,0x3c,0x77,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x40,0x00,
    0x00,0x02,0x50,0x88,0x53,0x1c,0x40,0x32,0x3c,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1e,0x40,0x1c,0x40,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x21,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x01,0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3d,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x7f,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xa5,0xb4,
};

constexpr uint8_t FILES_RESP1[] = {
    0x00,0x04,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x01,0x46,0x4f,0x52,0x54,0x59,0x74,0x77,0x6f,0x0f,0xff,0x00,
    0x04,0x00,0x00,0x02,0x00,0x4d,0x36,0x36,0x30,0x30,0x20,0x43,0x61,0x72,0x74,0x72,0x69,0x64,0x67,0x65,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x46,0x69,0x6c,0x65,0x30,0x30,0x31,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x7f,0xc6,0x8c,0x00,
    0x00,0x00,0x00,0xff,0x8d,0x16,0x90,0x30,0x31,0x30,0x31,0x32,0x30,0x31,0x39,0x30,0x30,0x30,0x31,0x30,0x38,
    0x30,0x30,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x30,0x30,0x30,0x31,0x33,0x32,0x30,0x30,0x46,0x69,0x6c,
    0x65,0x30,0x30,0x32,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7f,0xc8,0x00,0x00,0x00,
    0x00,0x00,0x00,0x7f,0xc6,0x8a,0x00,0x00,0x00,0x00,0xff,0x8d,0x13,0x10,0x30,0x31,0x30,0x31,0x32,0x30,0x31,
    0x39,0x30,0x30,0x30,0x32,0x30,0x37,0x30,0x30,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x30,0x30,0x30,0x32,0x33,0x31,0x30,0x30,0x46,0x69,0x6c,0x65,0x30,0x30,0x33,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x8f,0x00,0x00,0x00,0x00,0x00,0x00,0x7f,0xc6,0x8a,0x00,0x00,0x00,0x00,0xff,0x8d,0x13,0x10,0x30,0x31,0x30,0x31,0x32,0x30,0x31,0x39,0x30,0x30,0x30,0x33,0x34,0x36,0x30,0x30,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x30,0x30,0x30,0x34,0x31,0x30,0x30,0x30,0x46,0x69,0x6c,0x65,0x30,0x30,0x34,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x7f,0x56,0x00,0x00,0x00,0x00,0x00,0x00,0x7f,0xc6,0x8c,0x00,0x00,0x00,0x00,0xff,0x8d,0x16,0x90,0x30,0x31,0x30,0x31,0x32,0x30,0x31,0x39,0x30,0x30,0x30,0x31,0x30,0x39,0x30,0x30,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x30,0x30,0x30,0x31,0x33,0x33,0x30,0x30,
};

q_elem_t get_discovery_response() {
    q_elem_t elem;
    memcpy(elem.buf, DISCOVERY_RESP, sizeof(DISCOVERY_RESP));
    elem.len = sizeof(DISCOVERY_RESP);
    return elem;
}

TEST(Rmm, Constructor) {
    rmm_t rmm;
    ASSERT_TRUE(true);
}

TEST(Rmm, ParseDiscovery) {
    rmm_t rmm;

    q_elem_t elem = get_discovery_response();
    ASSERT_TRUE( rmm._parse_identity_response(&elem) );
    
    EXPECT_TRUE(rmm._serial_number.find("S251NXAH34") != std::string::npos);
    EXPECT_TRUE(rmm._model_number.find("EXM02B6QSamsung SSD 850 PRO 256GB") != std::string::npos);
}
