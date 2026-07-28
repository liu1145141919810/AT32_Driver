#include "RTOS_Demo.h"
#include "FreeRTOS.h"
#include "task.h"
#include "FSM.h"
#include "at32f423_gpio.h"
//=======   Tasks Area  ========
extern volatile uint32_t systemticks;
void workTask(void *arg){
    int cmd_idx=0;
    const int buf_size = 32;
    char cmd_buf[buf_size];
    CommandType command = DEFAULT;
    while(1){
        work_machine(&command, cmd_buf, buf_size, &cmd_idx);
        vTaskDelay(1 / portTICK_PERIOD_MS);//毫秒数处以每周期毫秒数，得到周期数
    }
}
//======= Interrupt Handlers ======
volatile uint32_t g_systick_tick_count = 0;

void SysTick_Handler(void)
{   
    systemticks++;
    xPortSysTickHandler();
}
void SVC_Handler(void)      { vPortSVCHandler(); }
void PendSV_Handler(void)   { xPortPendSVHandler(); }