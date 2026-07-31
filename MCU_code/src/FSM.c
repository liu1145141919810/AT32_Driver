#include <stddef.h>     // NULL definition
#include <string.h>     // strcmp, strncmp functions
#include <stdlib.h>     // atoi function
#include <stdio.h>      // sscanf function

#include "FreeRTOS.h"
#include "task.h" // These two header files need to be included together
#include "FSM.h"
#include "utility.h"
#include "init.h"
#include "at32f423_usart.h"
#include "at32f423_gpio.h"  // GPIO peripheral driver


//====================

/** @brief External Dependency Declaration */
extern void simple_read(void);
can_rx_message_type can_rx_msg;
// ======================================================
/** @brief Continuous State Function Declaration Section */
static void deFaultFunc(void);
static void OrderFunc(void);
static void lightFunc(void);
static void monitorFunc(void);
static void brightFunc(void);
/** @brief State Entry Function Declaration Section */
static void enterDefault(const char* cmd_buf);
static void enterOrder(const char* cmd_buf);
static void enterLight(const char* cmd_buf);
static void enterTemperature(const char* cmd_buf);
static void enterBright(const char* cmd_buf);
//===========================================
/** @brief Structure Definition Section */
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
    // Context variables are generally unnecessary for this architecture
    [DEFAULT]=deFaultFunc,
    [ORDER]=OrderFunc,
    [LIGHT]=lightFunc,
    [MONITOR]=monitorFunc,
    [BRIGHT]=brightFunc,
};

// Encapsulated helper function
static Event stringToEvent(const char* str){
    if(strcmp(str,"Order")==0)return GETIN_ORDER;
    if(strcmp(str,"Return")==0)return RETURN_DEFAULT;
    if(strcmp(str,"Light")==0)return ACT_LIGHT;
    if(strcmp(str,"Monitor")==0)return ACT_MONITOR;
    // Two valid formats for Bright command: "Bright 50" or standalone "Bright"
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
void fsm_conduct(CommandType *command){
    if(*command<ERROR_STATE && state_handlers[*command] != NULL){
        state_handlers[*command]();
    }
}
//=========================================
/** @brief State Entry Logic & Business Function Implementation*/
//=========
/** @brief Function Definition Section */
static void deFaultFunc(){
    static int hock = 0;
    static uint32_t last_toggle = 0;
    uint32_t now = xTaskGetTickCount();//Judgment completes instantly with no blocking; no task stall risk
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
/** @brief Command Parameter Parsing Functions */
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
//Further discussion required for this implementation section
static void pwm_set_duty(int percent,uint32_t pin){
    if(percent<0)percent=0;
    if(percent>100)percent=100;
    uint32_t ccr_value = (arr_value_read() * percent) / 100;
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
    // Reconfigure PA0/PA1/PA2 pins to alternate function mode for PWM
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
// External Function Interface Section
void usart0comm(char* cmd_buf, int buf_size, int* cmd_idx){//Echo function disabled
    int copy_len = uart_rx_len_read();
    if(copy_len>buf_size-1)copy_len=buf_size-1;
    for(int i = 0; i < copy_len; i++){
        cmd_buf[i] = usart_rx_buf_read(i);
    }
    cmd_buf[copy_len] = '\0'; // Append string terminator character
    *cmd_idx = copy_len;
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

void fsm_analysis(CommandType* command, char* cmd_buf, int buf_size, int* cmd_idx){
    cmd_buf[*cmd_idx - 1] = '\0'; *cmd_idx=0;// Remove trailing newline/return character
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
}
//=========== Data Access Interface Section ==========
can_rx_message_type* can_rx_msg_get(void){
    return &can_rx_msg;
}
