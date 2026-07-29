#include "RTOS_Demo.h"
#include "FSM.h"
#include "at32f423_gpio.h"
#include "init.h"
#define BUF_SIZE 32
//Public Acessing Area
static int cmd_idx=0;
static char cmd_buf[BUF_SIZE];//定义static数组必须use defined size
static CommandType command = DEFAULT;
//=====Interface Variable Area=====
TaskHandle_t usartTaskHandle;
TaskHandle_t canTaskHandle;
//=======   Tasks Area  ========
void FSMTask(void *arg){
    while(1){
        fsm_dealing(&command, cmd_buf, BUF_SIZE, &cmd_idx);
        vTaskDelay(1 / portTICK_PERIOD_MS);//毫秒数处以每周期毫秒数，得到周期数
    }
}
void UsartTask(void *arg){
    while(1){
        //等待UART事件
        ulTaskNotifyTake(
            pdTRUE,
            portMAX_DELAY
        );
        usart0comm(&command, cmd_buf, BUF_SIZE, &cmd_idx);
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}
void CanTask(void *arg){
    while(1){
        ulTaskNotifyTake(
            pdTRUE,
            portMAX_DELAY
        );
        canComm();
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}
//======= Interrupt Handlers ======
void SysTick_Handler(void)
{   
    xPortSysTickHandler();
}
void SVC_Handler(void)      { vPortSVCHandler(); }
void PendSV_Handler(void)   { xPortPendSVHandler(); }
void USART1_IRQHandler(void) {//Idle Detect Interrupt
    if(usart_flag_get(USART1, USART_IDLEF_FLAG) != RESET)
    {   
        usart_flag_clear(USART1, USART_IDLEF_FLAG);
        usart_data_receive(USART1);  // 读 DR 清除 IDLE 标志，除此外无用
        uart_rx_len = USART_RX_BUF_LEN - dma_data_number_get(DMA1_CHANNEL2);
        //==== FreeRTOS Task Notification ====
        BaseType_t wakeup = pdFALSE;
        vTaskNotifyGiveFromISR(usartTaskHandle, &wakeup);
        portYIELD_FROM_ISR(wakeup);
    }
}//CAN会自己管理边界，不需要重置
void CAN1_RX0_IRQHandler(void)
{//触发通信逻辑，这里是简单地提示存取，可以复杂化
    if (can_interrupt_flag_get(CAN1, CAN_RF0MN_FLAG) != RESET)
    {   
        can_flag_clear(CAN1, CAN_RF0MN_FLAG);
        can_message_receive(CAN1, CAN_RX_FIFO0, &can_rx_msg);
        //==== FreeRTOS Task Notification ====
        BaseType_t wakeup = pdFALSE;
        vTaskNotifyGiveFromISR(canTaskHandle, &wakeup);
        portYIELD_FROM_ISR(wakeup);
    }
}