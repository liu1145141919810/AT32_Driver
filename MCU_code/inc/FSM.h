#ifndef FSM_H
#define FSM_H
#include "at32f423_can.h"
#include "Msg_Protocol.h"
#include "Command_analyzer.h"
void usart0comm(char* cmd_buf, int buf_size, int* cmd_idx);
void usart_fsm_analysis(CommandType* command, char* cmd_buf, int buf_size, int* cmd_idx);
void can_fsm_analysis(CommandType* command,Command* event);
void fsm_conduct(CommandType* command);
//=========== Data Interface Area ==========
//Continuous Function Handlers
#endif // FSM_H