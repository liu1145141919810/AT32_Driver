#include "at32f423_can.h"

#include "can_protocol.h"
#include "canUtility.h"
#include "Msg_Protocol.h"
static can_rx_message_type can_rx_msg;
static void can1_send(uint32_t id, uint8_t *data, uint8_t len)
{
    can_tx_message_type tx_msg;
    tx_msg.standard_id = id;
    tx_msg.extended_id = 0;//Id for the extended frame, not used in this demo
    tx_msg.id_type = CAN_ID_STANDARD;
    tx_msg.frame_type = CAN_TFT_DATA;
    tx_msg.dlc = len;
    for (int i = 0; i < len && i < 8; i++)
        tx_msg.data[i] = data[i];
    can_message_transmit(CAN1, &tx_msg);
}
typedef struct{
    uint16_t id;
    Event event;
}Transition;
static Transition can_transition_table[]={
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
    for(int i=0;i<sizeof(can_transition_table)/sizeof(Transition);i++){
        if(can_transition_table[i].id==id){
            return can_transition_table[i].event;
        }
    }return ERROR_DEMO;
}
void analyze_can_msg(Command* cmd){
    cmd->event=can_id_to_event(can_rx_msg.standard_id);
    cmd->param_count=can_rx_msg.dlc;
    for(int i=0;i<cmd->param_count && i<8;i++){
        cmd->params[i]=can_rx_msg.data[i];
    }
}
void canSendBack(uint16_t id,uint8_t* data,uint8_t len){
    uint8_t resp[8];
    for(int i=0;i<len && i<8;i++) resp[i]=data[i];
    can1_send(id,resp,len);
}
//========== Interface Area =========
can_rx_message_type* can_rx_msg_get(void){
    //using for notify interrupt in FreeRTOS
    return &can_rx_msg;
}