#include "FreeRTOS.h"
#include "task.h"
#include "init.h"
#include "RTOS_Demo.h"
#include <stdio.h>
void initwork(){
    wait_for_power_stable();
    system_clock_config();  // 配置时钟到 144MHz，匹配 configCPU_CLOCK_HZ
    //systick_clock_source_config(SYSTICK_CLOCK_SOURCE_AHBCLK_NODIV);
    at32_board_init();//GPIO初始化                                         
    init_gpio_demo();//delay初始化
    uart_print_init(115200);
    ertc_init();
    systick_clock_source_config(SYSTICK_CLOCK_SOURCE_AHBCLK_NODIV);
    adc_config();
    dma_temp_velo_config();
    init_usart_rv_dma_demo();
    init_can1_demo();
        //修改一
    pwm_init();
    // 使能接收器
}
int main(void)
{   
    initwork();
    printf("\r\n\r\n\r\n");
    printf("\r\n====== AT32F423 USART Test ======\r\n");
    printf("Baudrate: 115200, 8N1\r\n");
    printf("Send any character to echo back\r\n");
    printf("LEDs will blink every 500ms\r\n");
    ertc_print_time();
    simple_read();
    printf(">");
    xTaskCreate(workTask, "work", 512, NULL, 1, NULL);
    vTaskStartScheduler();
    /* 不应该到这里 */
    while (1);
}