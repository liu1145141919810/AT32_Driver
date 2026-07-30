#ifndef RTOS_DEMO_H
#define RTOS_DEMO_H
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#define BUF_SIZE 32
//========== Demo Structure =========
typedef struct{
    char cmd_buf[BUF_SIZE];
    int len;
} CmdMessage;
//========Variable Area========
extern TaskHandle_t usartTaskHandle;//本身是指针，地址不会变化不用volatile
extern TaskHandle_t canTaskHandle;
extern QueueHandle_t cmdQueue;
//========Task Area========
void FSMTask(void *arg);
void UsartTask(void *arg);
void CanTask(void *arg);
//====== FreeRTOS Interrupt Handlers ======
void xPortSysTickHandler(void);
void vPortSVCHandler(void);
void xPortPendSVHandler(void);
//========Interrupt Handlers========
void SysTick_Handler(void);
void SVC_Handler(void);
void PendSV_Handler(void);
void USART1_IRQHandler(void);
#endif  //RTOS_DEMO_H