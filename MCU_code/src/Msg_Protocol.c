#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include "Msg_Protocol.h"
#include "LogOutUtility.h"

#include "public_define.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "at32f423_usart.h"
static QueueHandle_t printQueue;
//== Deal with the IO pointer ==
static void logout_raw(CommandType command, const char* buf,va_list args);
static void logout_queue(CommandType command,const char* buf,va_list args);
static void (*msgPrintPointer)(CommandType command,const char* payload,va_list args)=logout_raw;

void initPrintQueue(int length){
    printQueue = xQueueCreate(length, sizeof(Frame));
}
uint8_t QueueMsgReceive(Frame* frame){
    return (xQueueReceive(printQueue, frame, portMAX_DELAY)==pdPASS);
}
/*
=============CRC-8/MAXIM=======================
Width  = 8
Poly   = 0x31
        (Reflected implementation uses 0x8C)
Init   = 0x00
RefIn  = True
RefOut = True
XorOut = 0x00
================================================
*/
//void demoPrint(const char *format, ...);
uint8_t CRC8_MAXIM_calculate(uint8_t *data, uint16_t len)
{
    uint8_t crc = 0x00;
    for(uint16_t i = 0; i < len; i++)//data dealing
    {
        crc ^= data[i];
        for(uint8_t j = 0; j < 8; j++)//digit dealing
        {
            if(crc & 0x01)
                crc = (crc >> 1) ^ 0x8C;
            else
                crc >>= 1;
        }
    }return crc;
}
void msgPrint(CommandType command, const char* payload,...){
    if(msgPrintPointer==logout_raw&&command!=NOADDING){
        msgPrintPointer=logout_queue;
    }
    va_list args;
    va_start(args, payload);
    msgPrintPointer(command,payload,args);
    va_end(args);
}
//========= Area for Logout Raw ==================
static void logout_raw(CommandType command, const char* buf,va_list args){
    char buffer[4+PAYLOAD_MAX_LEN];
    buffer[0] = 0xAA;
    buffer[1] = command;
    buffer[2] = 0;//Must also clean the length issue
    demoPrint(buffer+3,&buffer[2],buf,args);
    buffer[3+buffer[2]] = CRC8_MAXIM_calculate((uint8_t*)buffer, 3+buffer[2]);
    for(int i=0;i<4+buffer[2];i++){
        while(usart_flag_get(PRINT_UART, USART_TDBE_FLAG) == RESET);
        usart_data_transmit(PRINT_UART, (uint16_t)(buffer[i]));
        while(usart_flag_get(PRINT_UART, USART_TDC_FLAG) == RESET);
    }
}
//========= Area for Logout Queue ==================
static void logout_queue(CommandType command, const char* payload,va_list args){
    Frame frame;
    frame.head = 0xAA;
    frame.type = command;
    frame.len = 0;
    
    char buffer[PAYLOAD_MAX_LEN];
    demoPrint(buffer,&frame.len,payload,args);
    
    if(frame.len==-1){
        memcpy(frame.payload, "Error: Payload length exceeds maximum limit of 32 bytes",
             strlen("Error: Payload length exceeds maximum limit of 32 bytes"));
    }
    else{
        memcpy(frame.payload, buffer, frame.len);
    }
    //frame.len = strlen(frame.payload);//Problem:it would include the undefined part, must use clean number
    frame.crc = CRC8_MAXIM_calculate((uint8_t*)&frame, 3+frame.len);//Calculate CRC8 for head, type, len, and payload
    //Send it into the Queue
    xQueueSend(printQueue, &frame, portMAX_DELAY);
}
uint8_t encapsulate_frame(char* buffer,Frame frame){
    buffer[0] = frame.head;
    buffer[1] = frame.type;
    buffer[2] = frame.len;
    memcpy(&buffer[3], frame.payload, frame.len);
    buffer[3 + frame.len] = frame.crc;
    return 4 + frame.len; // Total length of the encapsulated frame    
}