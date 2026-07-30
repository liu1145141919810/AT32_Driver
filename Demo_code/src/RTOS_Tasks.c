#include "RTOS_Tasks.h"
#include "FSM.h"
#include "queue.h"
#include "at32f423_gpio.h"
#include "utility.h"
#include "init.h"
#define BUF_SIZE 32
//Public Acessing Area

//=====Interface Variable Area=====
TaskHandle_t usartTaskHandle;
TaskHandle_t canTaskHandle;
QueueHandle_t cmdQueue;
//=======   Tasks Area  ========
void FSMTask(void *arg){
    CommandType command = DEFAULT;
    CmdMessage cmd;
    while(1){
        if(xQueueReceive(cmdQueue, &cmd, pdMS_TO_TICKS(0)) == pdPASS){
            fsm_analysis(&command, cmd.cmd_buf, BUF_SIZE, &cmd.len);
        }
        fsm_conduct(&command);
        vTaskDelay(1 / portTICK_PERIOD_MS);//ms count devide ms/clock,get clock count
        //This method let the delay more fixed
    }
}
void UsartTask(void *arg){
    CmdMessage cmd; 
    while(1){
        //等待UART事件
        ulTaskNotifyTake(
            pdTRUE,
            portMAX_DELAY
        );
        int xx=0;
        if(uart_rx_len_read()>0&&(usart_rx_buf_read(uart_rx_len_read()-1)=='\n'
        ||usart_rx_buf_read(uart_rx_len_read()-1)=='\r')){
            usart0comm(cmd.cmd_buf, BUF_SIZE, &cmd.len);
            xQueueSend(// Send the command to the FSM task via queue
                cmdQueue,
                &cmd,
                portMAX_DELAY
            );
        }
    }
}
void CanTask(void *arg){
    while(1){
        ulTaskNotifyTake(
            pdTRUE,
            portMAX_DELAY
        );
        canComm();
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
        //======== Hand typeing retreat area =========
        uart_rx_len_write(USART_RX_BUF_LEN - dma_data_number_get(DMA1_CHANNEL2));
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
        can_message_receive(CAN1, CAN_RX_FIFO0, can_rx_msg_get());
        //==== FreeRTOS Task Notification ====
        BaseType_t wakeup = pdFALSE;
        vTaskNotifyGiveFromISR(canTaskHandle, &wakeup);
        portYIELD_FROM_ISR(wakeup);
    }
}
