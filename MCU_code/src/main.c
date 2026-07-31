#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "init.h"
#include "utility.h"
#include "RTOS_Tasks.h"
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
    demoPrint("\r\n\r\n\r\n");
    demoPrint("\r\n====== AT32F423 USART Test ======\r\n");
    demoPrint("Baudrate: 115200, 8N1\r\n");
    demoPrint("Send any character to echo back\r\n");
    demoPrint("LEDs will blink every 500ms\r\n");
    ertc_print_time();
    simple_read();
}
void RTOSInit(void){
    xTaskCreate(FSMTask, "work", 512, NULL, 1, NULL);//Sole apply Sole Task
    xTaskCreate(UsartTask, "usart", 512, NULL, 2, &usartTaskHandle);
    xTaskCreate(CanTask, "can", 512, NULL, 2, &canTaskHandle);
    cmdQueue = xQueueCreate(5, sizeof(CmdMessage));
    vTaskStartScheduler();
}
int main(void)
{   
    initwork();
    demoPrint("AT32_READY");//Activate
    beginInfo();
    RTOSInit();
    /* Should never reach here */
    while (1);
}