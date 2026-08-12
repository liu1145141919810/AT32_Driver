#ifndef CAN_UTILITY_H
#define CAN_UTILITY_H
#include <stdint.h>
#include <Command_analyzer.h>
//=======Currently: only could be used by RTOS_Tasks.c
void analyze_can_msg(Command* cmd);
void canSendBack(uint16_t id, uint8_t *data, uint8_t len);
can_rx_message_type* can_rx_msg_get(void);
#endif // CAN_UTILITY_H