#include "RTOS_Tasks.h"
#include "FSM.h"
#include "queue.h"
#include "at32f423_gpio.h"
#include "utility.h"
#include "init.h"
#define BUF_SIZE 32
//Public Accessing Area

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
        vTaskDelay(1 / portTICK_PERIOD_MS);//Convert millisecond value to system ticks by dividing ms by tick period
        //This implementation enables more precise and stable delay timing
    }
}
void UsartTask(void *arg){
    CmdMessage cmd; 
    while(1){
        //Wait for UART event trigger
        ulTaskNotifyTake(
            pdTRUE,
            portMAX_DELAY
        );
        int xx=0;
        if(uart_rx_len_read()>0&&(usart_rx_buf_read(uart_rx_len_read()-1)=='\n'
        ||usart_rx_buf_read(uart_rx_len_read()-1)=='\r')){
            usart0comm(cmd.cmd_buf, BUF_SIZE, &cmd.len);
            xQueueSend(// Transmit command data to FSM task through message queue
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

void USART1_IRQHandler(void) {//UART Idle Line Detection Interrupt
    if(usart_flag_get(USART1, USART_IDLEF_FLAG) != RESET)
    {   
        usart_flag_clear(USART1, USART_IDLEF_FLAG);
        usart_data_receive(USART1);  //Read DR register to clear IDLE flag, no other functional purpose
        //======== Buffer Length Recalculation Region =========
        uart_rx_len_write(USART_RX_BUF_LEN - dma_data_number_get(DMA1_CHANNEL2));
        //==== FreeRTOS Task Notification Mechanism ====
        BaseType_t wakeup = pdFALSE;
        vTaskNotifyGiveFromISR(usartTaskHandle, &wakeup);
        portYIELD_FROM_ISR(wakeup);
    }
}//CAN peripheral manages frame boundary internally, no extra reset operation required

void CAN1_RX0_IRQHandler(void)
{//Trigger CAN communication processing logic; this is a basic notification mechanism and can be expanded with complex logic
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
