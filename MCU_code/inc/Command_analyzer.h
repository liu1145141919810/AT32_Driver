#ifndef COMMAND_ANALYZER_H
#define COMMAND_ANALYZER_H
#include "public_define.h"
#include "Msg_Protocol.h"
#include "typeBasement.h"
typedef struct{
    char cmd_buf[CMD_BUF_SIZE];
    int len;
} UsartCmdMessage;
typedef struct{
    Event event;
    int params[8];
    uint8_t param_count;
}Command;
Command parseCommand(char* cmd_buf);
#endif