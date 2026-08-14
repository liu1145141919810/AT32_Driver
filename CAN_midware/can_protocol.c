#include "can_protocol.h"
#include "typeBasement.h"

static Transition_R can_receive_table[]={
   {CMD_GETIN_ORDER, 0},
   {CMD_RETURN_DEFAULT, 1},
   {CMD_ACT_LIGHT, 2},
   {CMD_ACT_MONITOR, 3},
   {CMD_ACT_BRIGHT, 4},
   {CMD_SHIFT_CALIBRATE, 5},
   {CMD_OFF, 6},
   {CMD_ERROR_DEMO, 7},
};
static Transition_T can_send_table[]={
   { 0 ,RPT_DEFAULT},
   { 1 ,RPT_ORDER},
   { 2 ,RPT_LIGHT},
   { 3 ,RPT_MONITOR},
   { 4 ,RPT_BRIGHT},
   { 5 ,RPT_CALIBRATE},
   { 6 ,RPT_NOADDING},
   { 7 ,RPT_ERROR_EVENT},
   { 8 ,RPT_ERROR_STATE},
};
Event can_id_to_event(uint16_t id){
   for(int i=0;i<sizeof(can_receive_table)/sizeof(Transition_R);i++){
       if(can_receive_table[i].id==id){
           return can_receive_table[i].event;
       }
   }
   return ERROR_DEMO;
}
uint16_t can_command_to_id(CommandType command,uint8_t substate){
   for(int i=0;i<sizeof(can_send_table)/sizeof(Transition_T);i++){
       if(can_send_table[i].command==command){
           if (command==MONITOR&&substate!=0){
               if (substate==1)
                   return RPT_MONITOR_INTERNAL_TEMPERATURE;
               else if (substate==2)
                   return RPT_MONITOR_INTERNAL_VREF;
           }
           return can_send_table[i].id;
       }
   }
   return 0xFFFF;
}
