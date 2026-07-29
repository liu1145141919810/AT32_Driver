#ifndef FSM_H
#define FSM_H
typedef enum{
    DEFAULT,ORDER,LIGHT,MONITOR,BRIGHT,ERROR_STATE
}CommandType; 
void usart0comm(CommandType* command, char* cmd_buf, int buf_size, int* cmd_idx);
void canComm();
void fsm_dealing(CommandType* command, char* cmd_buf, int buf_size, int* cmd_idx);
//Continuous Function Handlers
#endif // FSM_H