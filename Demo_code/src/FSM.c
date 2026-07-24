#include "FSM.h"
#include "init.h"
#include "at32f423_usart.h"
#include "at32f423_gpio.h"  //gpio

#include <stddef.h>     // NULL
#include <string.h>     // strcmp, strncmp
#include <stdio.h>      // printf
#include <stdlib.h>     // atoi
//====================
/** @brief External Dependency*/
extern volatile uint32_t systemticks;
extern void simple_read(void);
extern uint32_t arr_value;

// ======================================================
/** @brief  Declaration Area for Continuous Function */
static void deFaultFunc(void);
static void OrderFunc(void);
static void lightFunc(void);
static void temperatureFunc(void);
static void brightFunc(void);
/** @brief  Declaration Area for Entering Function */
static void enterDefault(const char* cmd_buf);
static void enterOrder(const char* cmd_buf);
static void enterLight(const char* cmd_buf);
static void enterTemperature(const char* cmd_buf);
static void enterBright(const char* cmd_buf);
//===========================================
/** @brief  Structure Declaration */ 
typedef enum{
    DEFAULT,ORDER,LIGHT,TEMPERATURE,BRIGHT,ERROR_STATE
}CommandType; 
typedef enum {
    GETIN_ORDER, RETURN_DEFAULT, ACT_LIGHT,
    ACT_TEMPERATURE, ACT_BRIGHT, OFF, ERROR_DEMO
} Event;
typedef void (*state_enter_handler)(const char* cmd_buf);
static state_enter_handler enter_handlers[]={
    [DEFAULT]=enterDefault,
    [ORDER]=enterOrder,
    [LIGHT]=enterLight,
    [TEMPERATURE]=enterTemperature,
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
    {ORDER, ACT_TEMPERATURE, TEMPERATURE},
    {LIGHT, OFF, ORDER},
    {TEMPERATURE, OFF, ORDER},
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
    [TEMPERATURE]=temperatureFunc,
    [BRIGHT]=brightFunc,
};

//封装函数
static Event stringToEvent(const char* str){
    if(strcmp(str,"Order")==0)return GETIN_ORDER;
    if(strcmp(str,"Return")==0)return RETURN_DEFAULT;
    if(strcmp(str,"Light")==0)return ACT_LIGHT;
    if(strcmp(str,"Temp")==0)return ACT_TEMPERATURE;
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
    uint32_t now = systemticks;
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
static void temperatureFunc(void){
    static uint32_t last_toggle = 0;
    uint32_t now = systemticks;
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
    printf("Entering DEFAULT state\r\n");
}
static void enterOrder(const char* cmd_buf){
    printf("Entering ORDER state\r\n");
    wait_for_power_stable();
    init_gpio_demo();
}
static void enterLight(const char* cmd_buf){
    printf("Entering LIGHT state\r\n");
}
static void enterTemperature(const char* cmd_buf){
    printf("Entering TEMPERATURE state\r\n");
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
        printf("Error: Invalid Bright command format\r\n");
        printf("Getting into default Bright case\r\n");
    }
    printf("Entering BRIGHT state\r\n");
}
void work_machine(void){
    int cmd_idx=0;
    const int buf_size = 32;
    char cmd_buf[buf_size];
    CommandType command = DEFAULT;
    //===================================
    while (1) {
        if(uart_rx_done) {
            uart_rx_done = 0;
            // 拷贝接收到的数据到 cmd_buf（去掉末尾 \r\n）
            int len = uart_rx_len;
            if(len > buf_size - 1) len = buf_size - 1;
            for(int i = 0; i < len; i++){
                if(usart_rx_buf[i] == '\b'||usart_rx_buf[i]==0x7F){
                   if(cmd_idx > 0){
                    cmd_idx--;
                    printf(" \b");
                }
                } // 处理退格键
                else if(cmd_idx<buf_size - 1){
                    cmd_buf[cmd_idx++] = usart_rx_buf[i];
                }
                else{
                    printf("Error: Command buffer overflow, CLean All\r\n");
                    cmd_idx = 0; // 重置索引以避免溢出
                }
            }
            //这里做了手动化人工处理
            if(len > 0&& (cmd_buf[cmd_idx - 1] == '\n'|| cmd_buf[cmd_idx - 1] == '\r')) {
                cmd_buf[cmd_idx - 1] = '\0'; cmd_idx=0;// 去掉末尾的换行符
                Event event = stringToEvent(cmd_buf);
                if(event == ERROR_DEMO) {
                    printf("Error: Unrecognized command '%s'\r\n", cmd_buf);
                }
                else {
                    fsm_handle_event(&command, event);
                    if(command == ERROR_STATE) {
                        printf("Error: Invalid transition for command '%s'\r\n", cmd_buf);
                        printf("Recovering to DEFAULT state.\r\n");
                        command = DEFAULT;
                    }
                    else
                        fsm_enter(command, cmd_buf);
                }
                printf(">");
            }
            // 重置 DMA 接收位置
            reset_usart_dma();
        }

        fsm_execute(command);
        //现阶段:先这里最简单地写一个应答机制，后续可以考虑更复杂的处理
        if(can_rx_done){
            can_rx_done = 0;
            printf("\r\n[CAN RX] ID=0x%03lX DLC=%d",
                   can_rx_msg.standard_id, can_rx_msg.dlc);
            for(int i = 0; i < can_rx_msg.dlc; i++)
                printf(" %02X", can_rx_msg.data[i]);
            printf("\r\n>");
            uint8_t resp[8];
            resp[0] = can_rx_msg.data[0];
            resp[1] = 0xAB;
            resp[2] = 0xCD;
            can1_send(0x300, resp, 3);
        }
    }
}