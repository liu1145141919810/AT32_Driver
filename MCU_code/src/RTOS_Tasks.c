#include "at32f423_gpio.h"
#include "at32f423_dma.h"

#include "LogOutUtility.h"
#include "canUtility.h"
#include "init.h"
#include "Msg_Protocol.h"
#include "RTOS_Tasks.h"
#include "FSM.h"
#include "Command_analyzer.h"
#define CMD_BUF_SIZE 32
#define PRINT_DMA_BUF_SIZE 5
//Public Accessing Area

//=====Interface Variable Area=====
TaskHandle_t usartTaskHandle;
TaskHandle_t canTaskHandle;
TaskHandle_t usartTransmitTaskHandle;
QueueHandle_t UsartcmdQueue;
QueueHandle_t CancmdQueue;
void initcmdQueue(int length){
    UsartcmdQueue = xQueueCreate(length, sizeof(UsartCmdMessage));
    CancmdQueue = xQueueCreate(length, sizeof(Command));
}
//=======   Tasks Area  ========
void UsartTransmitTask(void *arg){//This is for usart output processing
    static char tx_buf[64];  // Buffer for encapsulated frame data
    Frame frame;
    initPrintDMA(tx_buf, PRINT_DMA_BUF_SIZE);//Set the buffer size to 5
    while(1){
        if(QueueMsgReceive(&frame)){
            // encapsulate: head + type + len + payload + crc
            uint8_t frame_len = encapsulate_frame(tx_buf, frame);

            dma_channel_enable(DMA1_CHANNEL3, FALSE);
            dma_flag_clear(DMA1_FDT3_FLAG);//Erase the flags
            dma_data_number_set(DMA1_CHANNEL3, frame_len);//regi write
            DMA1_CHANNEL3->maddr = (uint32_t)tx_buf;
            dma_channel_enable(DMA1_CHANNEL3, TRUE);

            // Must configure the dma function for usart-tx
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        }
    }
}
void FSMTask(void *arg){
    CommandType command = DEFAULT;
    UsartCmdMessage cmd;
    Command can_cmd;
    while(1){
        if(xQueueReceive(UsartcmdQueue, &cmd, pdMS_TO_TICKS(0)) == pdPASS){
            usart_fsm_analysis(&command, cmd.cmd_buf, CMD_BUF_SIZE, &cmd.len);
        }
        if(xQueueReceive(CancmdQueue, &can_cmd, pdMS_TO_TICKS(0)) == pdPASS){
            can_fsm_analysis(&command, &can_cmd);
        }
        fsm_conduct(&command);
        vTaskDelay(1 / portTICK_PERIOD_MS);//Convert millisecond value to system ticks by dividing ms by tick period
        //This implementation enables more precise and stable delay timing
    }
}
void UsartTask(void *arg){//This is for the usart input processing
    UsartCmdMessage cmd; 
    while(1){
        //Wait for UART event trigger
        ulTaskNotifyTake(
            pdTRUE,
            portMAX_DELAY
        );
        if(uart_rx_len_read()>0&&(usart_rx_buf_read(uart_rx_len_read()-1)=='\n'
        ||usart_rx_buf_read(uart_rx_len_read()-1)=='\r')){
            usart0comm(cmd.cmd_buf, CMD_BUF_SIZE, &cmd.len);
            if(cmd.len>0)
                xQueueSend(// Transmit command data to FSM task through message queue
                    UsartcmdQueue,
                    &cmd,
                    portMAX_DELAY
                );
            else if(cmd.len==0){
                // Discard the command if the length is zero
                msgPrint(ERROR_EVENT,"Error: Invalid command received, length is zero.");
            }
        }
    }
}
void CanTask(void *arg){
    Command cmd;
    while(1){
        ulTaskNotifyTake(
            pdTRUE,
            portMAX_DELAY
        );
    analyze_can_msg(&cmd);
    xQueueSend(
        CancmdQueue,
        &cmd,
        portMAX_DELAY
    );
    uint8_t resp[8];
    resp[1] = 0xAB;
    resp[2] = 0xCD;
    canSendBack(0x300, resp, 3);
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
//Design for the DMA finishing event

void DMA1_Channel3_IRQHandler(void)
{
    if (dma_interrupt_flag_get(DMA1_FDT3_FLAG) != RESET)//Could find the finish event flag for DMA1 Channel 3 in head file
    {
        dma_flag_clear(DMA1_FDT3_FLAG);
        //==== FreeRTOS Task Notification ====
        BaseType_t wakeup = pdFALSE;
        vTaskNotifyGiveFromISR(usartTransmitTaskHandle, &wakeup);
        portYIELD_FROM_ISR(wakeup);
    }
}