#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "init.h"
#include "Msg_Protocol.h"
#include "RTOS_Tasks.h"
#include "interrupt.h"
#include "utility.h"
#include "LogOutUtility.h"
#include "canUtility.h"
void startup_call(void);
void initwork(){
    system_clock_config();  //Configure system clock to 144 MHz, consistent with configCPU_CLOCK_HZ
    //systick_clock_source_config(SYSTICK_CLOCK_SOURCE_AHBCLK_NODIV);
    at32_board_init();//GPIO Init                                         
    init_gpio_demo();//delay Init
    uart_print_init(115200);
    ertc_init();
    adc_config();
    dma_temp_velo_config();
    init_usart_rv_dma_demo();
    init_can1_demo();
    pwm_init();
}
void beginInfo(void){
    CommandType print_state=NOADDING;
    //msgPrint(print_state,"\r\n\r\n\r\n");
    msgPrint(print_state,"AT32_READY\r\n");//Activate
    msgPrint(print_state,"====== AT32F423 USART Test ======");
    msgPrint(print_state,"Baudrate: 115200, 8N1");
    msgPrint(print_state,"Send any character to echo back");
    msgPrint(print_state,"LEDs will blink every 500ms");
    ertc_print_time(print_state);
    simple_read(print_state);
}
void RTOSInit(void){
    int size=5;
    initcmdQueue(size);
    initPrintQueue(size);
    initCanTransmitQueue(size);

    if(xTaskCreate(UsartTransmitTask,"usart_transmit",512,NULL,2,&usartTransmitTaskHandle) != pdPASS){
        // 任务创建失败！堆内存不足
        while(1);
    }
    if(xTaskCreate(CanTransmitTask,"can_transmit",512,NULL,2,NULL) != pdPASS){
        while(1);
    }
    if(xTaskCreate(FSMTask, "work", 512, NULL, 1, NULL) != pdPASS){//Sole apply Sole Task
        while(1);
    }
    if(xTaskCreate(UsartTask, "usart", 512, NULL, 2, &usartTaskHandle) != pdPASS){
        while(1);
    }
    if(xTaskCreate(CanTask, "can", 512, NULL, 2, &canTaskHandle) != pdPASS){
        while(1);
    }
    vTaskStartScheduler();
}
int main(void)
{   
    
    initwork();
    nvic_priority_config();
    msgPrint(NOADDING,"Waiting for initialization...\r\n");//
    msgPrint(NOADDING,"......\r\n");
    beginInfo();
    //==== Before the RTOS Area, all the print work is in the busy wait mode ====
    RTOSInit();
    /* Should never reach here */
    while (1);
}