#include "print_utils.h"

void print_buf(const uint8_t* buf, uint32_t len) {
    printf("%p\n", buf);
    for (uint32_t i = 0; i < len; i++) {
        printf("0x%02x,", buf[i]);
    }

    printf("\n");
}
