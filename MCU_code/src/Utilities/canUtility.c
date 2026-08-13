#include<stdarg.h>

#include "FreeRTOS.h"
#include "queue.h"

#include "can_protocol.h"
#include "canUtility.h"
#include "Msg_Protocol.h"
static QueueHandle_t CanTransmitQueue;
static can_rx_message_type can_rx_msg;//Used for receiving the information of can

void initCanTransmitQueue(int length){
    CanTransmitQueue = xQueueCreate(length, sizeof(can_tx_message_type));
}
uint8_t canTransmitQueueSend(can_tx_message_type* tx_msg){
    return (xQueueReceive(CanTransmitQueue, tx_msg, portMAX_DELAY)==pdPASS);
}

static can_tx_message_type can1_prepare(uint32_t id, uint8_t *data, uint8_t len)
{
    can_tx_message_type tx_msg;
    tx_msg.standard_id = id;
    tx_msg.extended_id = 0;//Id for the extended frame, not used in this demo
    tx_msg.id_type = CAN_ID_STANDARD;
    tx_msg.frame_type = CAN_TFT_DATA;
    tx_msg.dlc = len;
    for (int i = 0; i < len && i < 8; i++)
        tx_msg.data[i] = data[i];
    return tx_msg;
}
//======== Receive Table for Can =========
typedef struct{
    uint16_t id;
    Event event;
}Transition_R;
static Transition_R can_receive_table[]={
    {CMD_ORDER, GETIN_ORDER},
    {CMD_RETURN, RETURN_DEFAULT},
    {CMD_LIGHT, ACT_LIGHT},
    {CMD_MONITOR, ACT_MONITOR},
    {CMD_BRIGHT, ACT_BRIGHT},
    {CMD_CALIBRATE, SHIFT_CALIBRATE},
    {CMD_OFF, OFF},
    {CMD_ERROR_DEMO, ERROR_DEMO}
};
static Event can_id_to_event(uint16_t id){
    for(int i=0;i<sizeof(can_receive_table)/sizeof(Transition_R);i++){
        if(can_receive_table[i].id==id){
            return can_receive_table[i].event;
        }
    }return ERROR_DEMO;
}
//======== Send Table for Can =========
typedef struct{
    CommandType command;
    uint16_t id;
}Transition_T;
static Transition_T can_send_table[]={
    {DEFAULT,RPT_DEFAULT},
    {ORDER,RPT_ORDER},
    {LIGHT,RPT_LIGHT},
    {MONITOR,RPT_MONITOR},
    {BRIGHT,RPT_BRIGHT},
    {CALIBRATE,RPT_CALIBRATE},
    {NOADDING,RPT_NOADDING},
    {ERROR_EVENT,RPT_ERROR_EVENT},
    {ERROR_STATE,RPT_ERROR_STATE}
};
static uint16_t can_command_to_id(CommandType command,uint8_t substate){
    for(int i=0;i<sizeof(can_send_table)/sizeof(Transition_T);i++){
        if(can_send_table[i].command==command){
            if (command==MONITOR&&substate==1){
                if (substate==1)
                    return RPT_MONITOR_INTERNAL_TEMPERATURE;
                else if (substate==2)
                    return RPT_MONITOR_INTERNAL_VREF;
            }
            return can_send_table[i].id;
        }
    }return 0xFFFF;
}
void analyze_can_msg(Command* cmd){
    cmd->event=can_id_to_event(can_rx_msg.standard_id);
    cmd->param_count=can_rx_msg.dlc;
    for(int i=0;i<cmd->param_count && i<8;i++){
        cmd->params[i]=can_rx_msg.data[i];
    }
}
// This function could only be used by FSMTask series
void canSendBack(CommandType command,int arg_num,...){
    va_list args;
    va_start(args, arg_num);

    uint8_t data[8];data[0]=0;
    uint8_t len=arg_num>8?8:arg_num;
    for(int i=0;i<len;i++){
        data[i]=(uint8_t)va_arg(args,int);
    }
    uint16_t id=can_command_to_id(command, data[0]);
    can_tx_message_type tx_msg=can1_prepare(id,data,len);
    xQueueSend(
        CanTransmitQueue,
        &tx_msg,
        portMAX_DELAY
    );
    va_end(args);
}
//========== Interface Area =========
can_rx_message_type* can_rx_msg_get(void){
    //using for notify interrupt in FreeRTOS
    return &can_rx_msg;
}