#include <stddef.h>     // NULL
#include <string.h>     // strcmp, strncmp
#include <stdlib.h>     // atoi
#include <stdio.h>      // sscanf

#include "FreeRTOS.h"
#include "task.h" //这两个貌似要一起引用
#include "FSM.h"
#include "utility.h"
#include "init.h"
#include "at32f423_usart.h"
#include "at32f423_gpio.h"  //gpio


//====================
/** @brief External Dependency*/
extern void simple_read(void);
extern uint32_t arr_value;

// ======================================================
/** @brief  Declaration Area for Continuous Function */
static void deFaultFunc(void);
static void OrderFunc(void);
static void lightFunc(void);
static void monitorFunc(void);
static void brightFunc(void);
/** @brief  Declaration Area for Entering Function */
static void enterDefault(const char* cmd_buf);
static void enterOrder(const char* cmd_buf);
static void enterLight(const char* cmd_buf);
static void enterTemperature(const char* cmd_buf);
static void enterBright(const char* cmd_buf);
//===========================================
/** @brief  Structure Declaration */ 
typedef enum {
    GETIN_ORDER, RETURN_DEFAULT, ACT_LIGHT,
    ACT_MONITOR, ACT_BRIGHT, OFF, ERROR_DEMO
} Event;
typedef void (*state_enter_handler)(const char* cmd_buf);
static state_enter_handler enter_handlers[]={
    [DEFAULT]=enterDefault,
    [ORDER]=enterOrder,
    [LIGHT]=enterLight,
    [MONITOR]=enterTemperature,
    [BRIGHT]=enterBright,
};
typedef struct{
    CommandType from;
    Event event;
    CommandType to;
} Transition;
static Transition transition_table[]={
    {DEFAULT, GETIN_ORDER, ORDER},
    {ORDER, RETURN_DEFAULT, DEFAULT},
    {ORDER, ACT_LIGHT, LIGHT},
    {ORDER, ACT_MONITOR, MONITOR},
    {LIGHT, OFF, ORDER},
    {MONITOR, OFF, ORDER},
    {ORDER, ACT_BRIGHT, BRIGHT},
    {BRIGHT, OFF, ORDER},
    {BRIGHT, ACT_BRIGHT, BRIGHT},
};
typedef void (*state_handler_t)(void);
static state_handler_t state_handlers[]={
    //通常这种结构不需要设置上下文
    [DEFAULT]=deFaultFunc,
    [ORDER]=OrderFunc,
    [LIGHT]=lightFunc,
    [MONITOR]=monitorFunc,
    [BRIGHT]=brightFunc,
};

//封装函数
static Event stringToEvent(const char* str){
    if(strcmp(str,"Order")==0)return GETIN_ORDER;
    if(strcmp(str,"Return")==0)return RETURN_DEFAULT;
    if(strcmp(str,"Light")==0)return ACT_LIGHT;
    if(strcmp(str,"Monitor")==0)return ACT_MONITOR;
    //two type of Bright command: "Bright 50" or "Bright"
    if(strncmp(str, "Bright ", 7) == 0)return ACT_BRIGHT;
    if(strcmp(str,"Bright")==0)return ACT_BRIGHT;
    if(strcmp(str,"Off")==0)return OFF;
    return ERROR_DEMO;
}
static void fsm_handle_event(CommandType* current_state,Event event){
    for(int i=0;i<sizeof(transition_table)/sizeof(Transition);i++){
        if(transition_table[i].from==*current_state && transition_table[i].event==event){
            *current_state=transition_table[i].to;
            return;
        }
    }
    *current_state=ERROR_STATE;    
}
static void fsm_enter(CommandType command,const char* cmd_buf){
    if(command<ERROR_STATE && enter_handlers[command] != NULL){
        enter_handlers[command](cmd_buf);
    }
}
static void fsm_execute(CommandType command){
    if(command<ERROR_STATE && state_handlers[command] != NULL){
        state_handlers[command]();
    }
}
//=========================================
/** @brief Entering and Func realization*/
//=========
/** @brief Definition Area */
static void deFaultFunc(){
    static int hock = 0;
    static uint32_t last_toggle = 0;
    uint32_t now = xTaskGetTickCount();//判断瞬间完成，无任何阻塞，不用担心
    if (now - last_toggle >= 1000) {
        last_toggle = now;
        if(hock==0)
            gpio_bits_toggle(GPIOA, GPIO_PINS_0);
        else if(hock==1)
            gpio_bits_toggle(GPIOA, GPIO_PINS_1);
        else if(hock==2)
            gpio_bits_toggle(GPIOA, GPIO_PINS_2);
        hock=(hock+1)%3;
    }
}
static void OrderFunc(void){
    gpio_bits_set(GPIOA, GPIO_PINS_0 | GPIO_PINS_1 | GPIO_PINS_2);
}
static void lightFunc(void){
    gpio_bits_reset(GPIOA, GPIO_PINS_0 | GPIO_PINS_1 | GPIO_PINS_2);
}
static void monitorFunc(void){
    static uint32_t last_toggle = 0;
    uint32_t now = xTaskGetTickCount();
    if(now - last_toggle >= 4000) {
        last_toggle = now;
        simple_read();
    }
}
static void brightFunc(void){
    gpio_bits_reset(GPIOA, GPIO_PINS_0 | GPIO_PINS_1 | GPIO_PINS_2);
}
//=========
/** @brief Parameter Analysis */
static void enterDefault(const char* cmd_buf){
    demoPrint("Entering DEFAULT state\r\n");
}
static void enterOrder(const char* cmd_buf){
    demoPrint("Entering ORDER state\r\n");
    wait_for_power_stable();
    init_gpio_demo();
}
static void enterLight(const char* cmd_buf){
    demoPrint("Entering LIGHT state\r\n");
}
static void enterTemperature(const char* cmd_buf){
    demoPrint("Entering MONITORING state\r\n");
}
//It's needs more discussion here
static void pwm_set_duty(int percent,uint32_t pin){
    if(percent<0)percent=0;
    if(percent>100)percent=100;
    uint32_t ccr_value = (arr_value * percent) / 100;
    if(pin==GPIO_PINS_0){
        tmr_channel_value_set(TMR2, TMR_SELECT_CHANNEL_1, ccr_value);
    }
    else if(pin==GPIO_PINS_1){
        tmr_channel_value_set(TMR2, TMR_SELECT_CHANNEL_2, ccr_value);
    }
    else if(pin==GPIO_PINS_2){
        tmr_channel_value_set(TMR2, TMR_SELECT_CHANNEL_3, ccr_value);
    }
}
static void enterBright(const char* cmd_buf){
    // 切换 PA0/PA1/PA2 到复用模式
    shift_pwn_mode(GPIO_PINS_0 | GPIO_PINS_1 | GPIO_PINS_2);

    int b_value1,b_value2,b_value3,trash;
    int matched = sscanf(cmd_buf, "Bright %d %d %d %d", &b_value1, &b_value2, &b_value3, &trash);
    if(matched==-1){
        pwm_set_duty(0, GPIO_PINS_0);
    }
    else if(matched==1){
        pwm_set_duty(b_value1, GPIO_PINS_0);
    }
    else if(matched==2){
        pwm_set_duty(b_value1, GPIO_PINS_0);
        pwm_set_duty(b_value2, GPIO_PINS_1);
    }
    else if(matched==3){
        pwm_set_duty(b_value1, GPIO_PINS_0);
        pwm_set_duty(b_value2, GPIO_PINS_1);
        pwm_set_duty(b_value3, GPIO_PINS_2);
    }
    else{
        demoPrint("Error: Invalid Bright command format\r\n");
        demoPrint("Getting into default Bright case\r\n");
    }
    demoPrint("Entering BRIGHT state\r\n");
}
// Outter Interface Area
void usart0comm(CommandType* command, char* cmd_buf, int buf_size, int* cmd_idx){
    // 拷贝接收到的数据到 cmd_buf（去掉末尾 \r\n）
    int len = uart_rx_len;
    if(len > buf_size - 1) len = buf_size - 1;

    for(int i = 0; i < len; i++){
        if(usart_rx_buf[i] == '\b'||usart_rx_buf[i]==0x7F){
            if(*cmd_idx > 0){
            (*cmd_idx)--;
            }
        } // 处理退格键
        else if(*cmd_idx<buf_size - 1){
            cmd_buf[(*cmd_idx)++] = usart_rx_buf[i];
        }
        else{
            demoPrint("Error: Command buffer overflow, CLean All\r\n");
            *cmd_idx = 0; // 重置索引以避免溢出
        }
    }

    for(int i = 0; i < len; i++){
        if(usart_rx_buf[i] == '\b'||usart_rx_buf[i]==0x7F){
            demoPrint(" \b");
        }
    }
    reset_usart_dma();
}

void canComm(){
            demoPrint("\r\n[CAN RX] ID=0x%03lX DLC=%d",
                   can_rx_msg.standard_id, can_rx_msg.dlc);
            for(int i = 0; i < can_rx_msg.dlc; i++)
                demoPrint(" %02X", can_rx_msg.data[i]);
            demoPrint("\r\n>");
            uint8_t resp[8];
            resp[0] = can_rx_msg.data[0];
            resp[1] = 0xAB;
            resp[2] = 0xCD;
            can1_send(0x300, resp, 3);
}

void fsm_dealing(CommandType* command, char* cmd_buf, int buf_size, int* cmd_idx){
    if((*cmd_idx) > 0&& (cmd_buf[*cmd_idx - 1] == '\n'|| cmd_buf[*cmd_idx - 1] == '\r')) {
                cmd_buf[*cmd_idx - 1] = '\0'; *cmd_idx=0;// 去掉末尾的换行符
                Event event = stringToEvent(cmd_buf);
                if(event == ERROR_DEMO) {
                    demoPrint("Error: Unrecognized command %s\r\n", cmd_buf);
                }
                else {
                    fsm_handle_event(command, event);
                    if(*command == ERROR_STATE) {
                        demoPrint("Error: Invalid transition for command %s\r\n", cmd_buf);
                        demoPrint("Recovering to DEFAULT state.\r\n");
                        *command = DEFAULT;
                    }
                    else
                        fsm_enter(*command, cmd_buf);
                }
                demoPrint(">");
            }
    fsm_execute(*command);
}