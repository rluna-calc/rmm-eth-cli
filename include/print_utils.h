#ifndef _PRINT_UTILS_H_
#define _PRINT_UTILS_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>


void print_buf(const uint8_t* buf, uint32_t len, bool is_hex=false);

#endif