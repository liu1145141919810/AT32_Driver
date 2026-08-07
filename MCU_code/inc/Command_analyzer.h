#ifndef COMMAND_ANALYZER_H
#define COMMAND_ANALYZER_H
#include "public_define.h"
typedef struct{
    char cmd_buf[CMD_BUF_SIZE];
    int len;
} CmdMessage;
#endif