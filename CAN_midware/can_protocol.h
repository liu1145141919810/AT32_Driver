#ifndef CAN_PROTOCOL_H
//This file is auto-generated, and could only be used by canUtility.c

#include "Msg_Protocol.h"
#include "Command_analyzer.h"

#define CAN_PROTOCOL_H

typedef struct {
    CommandType command;
    uint16_t id;
} Transition_T;
typedef struct {
    uint16_t id;
    Event event;
} Transition_R;
Event can_id_to_event(uint16_t id);
uint16_t can_command_to_id(CommandType command,uint8_t substate);
#endif // CAN_PROTOCOL_H
