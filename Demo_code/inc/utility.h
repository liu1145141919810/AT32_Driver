#ifndef UTILITY_H
#define UTILITY_H
#include <stdint.h>
void simple_read(void);
void ertc_print_time(void);
void shift_pwn_mode(uint32_t pin);
void can1_send(uint32_t id, uint8_t *data, uint8_t len);
void demoPrint(const char* str, ...);
#endif