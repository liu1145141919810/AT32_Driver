#ifndef COMMAND_ANALYZER_H
#define COMMAND_ANALYZER_H
#include "public_define.h"
#include "Msg_Protocol.h"
typedef enum {
    GETIN_ORDER, RETURN_DEFAULT, ACT_LIGHT,
    ACT_MONITOR, ACT_BRIGHT,SHIFT_CALIBRATE ,OFF, ERROR_DEMO
} Event;
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