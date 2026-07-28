#ifndef RTOS_DEMO_H
#define RTOS_DEMO_H
//========Task Area========
void workTask(void *arg);
//====== FreeRTOS Interrupt Handlers ======
void xPortSysTickHandler(void);
void vPortSVCHandler(void);
void xPortPendSVHandler(void);
//========Interrupt Handlers========
void SysTick_Handler(void);
void SVC_Handler(void);
void PendSV_Handler(void);
#endif // RTOS_DEMO_H