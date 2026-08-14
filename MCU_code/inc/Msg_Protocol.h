#ifndef MSG_PROTOCOL_H
#define MSG_PROTOCOL_H
#include <stdint.h>
#include "typeBasement.h"
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
void initPrintQueue(int length);
uint8_t QueueMsgReceive(Frame* frame);
//====== IO Logic Unit ======
void msgPrint(CommandType command, const char* payload,...);
uint8_t encapsulate_frame(char* buffer,Frame frame);
uint8_t CRC8_MAXIM_calculate(uint8_t *data, uint16_t len);
#endif