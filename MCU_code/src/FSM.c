#include <stddef.h>     // NULL definition
#include <string.h>     // strcmp, strncmp functions
#include <stdlib.h>     // atoi function
#include <stdio.h>      // sscanf function

#include "FreeRTOS.h"
#include "task.h" // These two header files need to be included together
#include "at32f423_usart.h"
#include "at32f423_gpio.h"  // GPIO peripheral driver

#include "FSM.h"
#include "LogOutUtility.h"
#include "init.h"
#include "utility.h"  // Utility functions for hardware peripherals
#include "Command_analyzer.h"
#include "public_define.h"  // Public macro definitions

//====================

/** @brief External Dependency Declaration */
extern void simple_read(CommandType print_state);
can_rx_message_type can_rx_msg;
// ======================================================
/** @brief Continuous State Function Declaration Section */
static void deFaultFunc(void);
static void OrderFunc(void);
static void lightFunc(void);
static void monitorFunc(void);
static void brightFunc(void);
static void calibrateFunc(void);
/** @brief State Entry Function Declaration Section */
static void enterDefault(Command event);
static void enterOrder(Command event);
static void enterLight(Command event);
static void enterMonitor(Command event);
static void enterBright(Command event);
static void enterCalibrate(Command event);
//===========================================
/** @brief Structure Definition Section */

typedef void (*state_enter_handler)(Command event);
static state_enter_handler enter_handlers[]={
    [DEFAULT]=enterDefault,
    [ORDER]=enterOrder,
    [LIGHT]=enterLight,
    [MONITOR]=enterMonitor,
    [BRIGHT]=enterBright,
    [CALIBRATE]=enterCalibrate,
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
    {DEFAULT, SHIFT_CALIBRATE, CALIBRATE},
    {CALIBRATE, SHIFT_CALIBRATE, DEFAULT},
};
typedef void (*state_handler_t)(void);
static state_handler_t state_handlers[]={
    // Context variables are generally unnecessary for this architecture
    [DEFAULT]=deFaultFunc,
    [ORDER]=OrderFunc,
    [LIGHT]=lightFunc,
    [MONITOR]=monitorFunc,
    [BRIGHT]=brightFunc,
    [CALIBRATE]=calibrateFunc,
};

static void fsm_handle_event(CommandType* current_state,Event event){
    for(int i=0;i<sizeof(transition_table)/sizeof(Transition);i++){
        if(transition_table[i].from==*current_state && transition_table[i].event==event){
            *current_state=transition_table[i].to;
            return;
        }
    }
    *current_state=ERROR_STATE;
}
static void fsm_enter(CommandType command,Command event){
    if(command<ERROR_STATE && enter_handlers[command] != NULL){
        enter_handlers[command](event);
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
    if(now - last_toggle >= CALIBRATE_MONITOR_DELAY) {
        last_toggle = now;
        simple_read(MONITOR);
    }
}
static void brightFunc(void){
    gpio_bits_reset(GPIOA, GPIO_PINS_0 | GPIO_PINS_1 | GPIO_PINS_2);
}
static void calibrateFunc(void){
}

//=========
/** @brief Command Parameter Parsing Functions */
static void enterDefault(Command event){
    msgPrint(DEFAULT,"Entering DEFAULT state");
}
static void enterOrder(Command event){
    msgPrint(ORDER,"Entering ORDER state");
    wait_for_power_stable();
    init_gpio_demo();
}
static void enterLight(Command event){
    msgPrint(LIGHT,"Entering LIGHT state");
}
static void enterMonitor(Command event){
    msgPrint(MONITOR,"Entering MONITOR state");
}

static void enterCalibrate(Command event){
    msgPrint(CALIBRATE,"Entering CALIBRATE state");
    // Implement calibration logic here
    if(event.param_count!=7){
        msgPrint(ERROR_EVENT,"Error: Invalid number of parameters for calibration");
        return;
    }
    config_time(event.params[0], event.params[1], event.params[2],
                event.params[3], event.params[4], event.params[5], event.params[6]);
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

static void enterBright(Command event){
    // Reconfigure PA0/PA1/PA2 pins to alternate function mode for PWM
    shift_pwn_mode(GPIO_PINS_0 | GPIO_PINS_1 | GPIO_PINS_2);

    int b_value1,b_value2,b_value3,trash;
    if(event.param_count == 0){
        pwm_set_duty(0, GPIO_PINS_0);
    }
    else if(event.param_count == 1){
        pwm_set_duty(event.params[0], GPIO_PINS_0);
    }
    else if(event.param_count == 2){
        pwm_set_duty(event.params[0], GPIO_PINS_0);
        pwm_set_duty(event.params[1], GPIO_PINS_1);
    }
    else if(event.param_count == 3){
        pwm_set_duty(event.params[0], GPIO_PINS_0);
        pwm_set_duty(event.params[1], GPIO_PINS_1);
        pwm_set_duty(event.params[2], GPIO_PINS_2);
    }
    else{
        msgPrint(ERROR_EVENT,"Error: Invalid Bright command format");
        msgPrint(ERROR_EVENT,"Getting into default Bright case");
    }
    msgPrint(BRIGHT,"Entering BRIGHT state");
}

// External Function Interface Section
void usart0comm(char* cmd_buf, int buf_size, int* cmd_idx){//Echo function disabled

    //revise it to accept the frame
    *cmd_idx = 0;
    if (usart_rx_buf_read(0) == 0xAA) {
        // Process the received frame
        int state = usart_rx_buf_read(1); // Extract the state from the frame
        int len = usart_rx_buf_read(2);   // Extract the length of the payload

        char calculate[PAYLOAD_MAX_LEN+3];
        calculate[0] = 0xAA;
        calculate[1] = state;
        calculate[2] = len;

        if (len > 0 && len <= PAYLOAD_MAX_LEN) {
            for (int i = 0; i < len; i++){
                cmd_buf[i] = usart_rx_buf_read(3 + i); // Copy the
                calculate[i + 3] = cmd_buf[i]; 
            }
        }
        cmd_buf[len] = '\0'; // Append string terminator character
        *cmd_idx=len+1;
        uint8_t crc=CRC8_MAXIM_calculate((uint8_t*)calculate, len + 3);
        if (crc!=usart_rx_buf_read(3+len)){
            //it would be directly discard
            *cmd_idx = 0;
        }
    }
    //Otherwise, it would be directly without anything
    reset_usart_dma();
}

void canComm(){

    msgPrint(NOADDING,"\r\n[CAN RX] ID=0x%03lX DLC=%d",
            can_rx_msg.standard_id, can_rx_msg.dlc);
    for(int i = 0; i < can_rx_msg.dlc; i++)
        msgPrint(NOADDING, " %02X", can_rx_msg.data[i]);
    msgPrint(NOADDING,"\r\n>");
    uint8_t resp[8];
    resp[0] = can_rx_msg.data[0];
    resp[1] = 0xAB;
    resp[2] = 0xCD;
    can1_send(0x300, resp, 3);
}

void fsm_analysis(CommandType* command, char* cmd_buf, int buf_size, int* cmd_idx){
    cmd_buf[*cmd_idx - 1] = '\0'; *cmd_idx=0;// Remove trailing newline/return character
    Command event = parseCommand(cmd_buf);
    if(event.event == ERROR_DEMO) {
        msgPrint(ERROR_EVENT,"Error: Unrecognized command %s", cmd_buf);
    }
    else {
        fsm_handle_event(command, event.event);//Next command state get
        if(*command == ERROR_STATE) {
            msgPrint(ERROR,"Error: Invalid transition for command %s", cmd_buf);
            msgPrint(DEFAULT,"Recovering to DEFAULT state.");
            *command = DEFAULT;
        }
        else
            fsm_enter(*command, event);
    }
}
//=========== Data Access Interface Section ==========
can_rx_message_type* can_rx_msg_get(void){
    return &can_rx_msg;
}