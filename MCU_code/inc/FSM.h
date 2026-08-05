#ifndef FSM_H
#define FSM_H
#include "at32f423_can.h"
#include "Msg_Protocol.h"
void usart0comm(char* cmd_buf, int buf_size, int* cmd_idx);
void canComm();
void fsm_analysis(CommandType* command, char* cmd_buf, int buf_size, int* cmd_idx);
void fsm_conduct(CommandType* command);
//=========== Data Interface Area ==========
can_rx_message_type* can_rx_msg_get(void);
//Continuous Function Handlers
#endif // FSM_H