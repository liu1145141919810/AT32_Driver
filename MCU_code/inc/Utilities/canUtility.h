#ifndef CAN_UTILITY_H
#define CAN_UTILITY_H
#include "at32f423_can.h"
#include <stdint.h>
#include <Command_analyzer.h>
//=======Currently: only could be used by RTOS_Tasks.c
void initCanTransmitQueue(int length);
uint8_t canTransmitQueueSend(can_tx_message_type* tx_msg);
void analyze_can_msg(Command* cmd);
void canSendBack(CommandType command,int arg_num,...);
can_rx_message_type* can_rx_msg_get(void);
#endif // CAN_UTILITY_H