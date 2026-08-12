#ifndef UTILITY_H
#define UTILITY_H
#include <stdint.h>
#include "Msg_Protocol.h"
void simple_read(CommandType print_state);
void ertc_print_time(CommandType print_state);
void shift_pwn_mode(uint32_t pin);
#endif