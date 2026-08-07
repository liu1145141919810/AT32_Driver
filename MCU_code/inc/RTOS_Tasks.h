#ifndef RTOS_DEMO_H
#define RTOS_DEMO_H
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
//========== Demo Structure =========
typedef void (*Callback)(void);
//========Variable Area========
extern TaskHandle_t usartTransmitTaskHandle;
extern TaskHandle_t usartTaskHandle;//itself is a pointer, the address will not change, no need for volatile
extern TaskHandle_t canTaskHandle;
extern QueueHandle_t cmdQueue;
void initcmdQueue(int length);
//========Task Area========
void UsartTransmitTask(void *arg);
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