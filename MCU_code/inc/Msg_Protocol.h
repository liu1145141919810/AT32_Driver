#ifndef MSG_PROTOCOL_H
#define MSG_PROTOCOL_H
#include <stdint.h>
#define POLY 0x07
#define PAYLOAD_MAX_LEN 64
#define FRAME_MAX_LEN (PAYLOAD_MAX_LEN + 5) // 1 byte for head, 1 byte for type, 1 byte for len, 1 byte for crc
typedef struct{//Maximum is 64 bytes of payload and 69 bytes of total
    uint8_t head;
    uint8_t type;
    uint8_t len;
    uint8_t payload[PAYLOAD_MAX_LEN];
    uint8_t crc;
}Frame;
typedef enum{
    DEFAULT,ORDER,LIGHT,MONITOR,BRIGHT,CALIBRATE,NOADDING,ERROR_EVENT,ERROR_STATE
}CommandType; //The deep realize is digit starting from 0
void initPrintQueue(int length);
uint8_t QueueMsgReceive(Frame* frame);
//====== Print Logic Unit ======
void msgPrint(CommandType command, const char* payload,...);
uint8_t encapsulate_frame(char* buffer,Frame frame);
#endif