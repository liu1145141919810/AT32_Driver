#ifndef LOG_OUT_UTILITY_H
#define LOG_OUT_UTILITY_H
#include <stdint.h>
#include <stdarg.h>
void switchPrintMode(int mode);
uint8_t demoPrint(char* buffer, uint8_t* idx, const char* str, va_list args);
#endif // LOG_OUT_UTILITY_H