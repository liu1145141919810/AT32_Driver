#include "FreeRTOS.h"
#include "task.h"
#include "at32f423_gpio.h"
#include "at32f423_crm.h"
#include "at32f423_misc.h"

static void gpio_init_led(void)
{
    gpio_init_type gpio;
    crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
    gpio.gpio_mode = GPIO_MODE_OUTPUT;
    gpio.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio.gpio_pull = GPIO_PULL_NONE;
    gpio.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio.gpio_pins = GPIO_PINS_0 | GPIO_PINS_1 | GPIO_PINS_2;
    gpio_init(GPIOA, &gpio);
}

/* 诊断步骤：
   步骤1: main 里直接操作 GPIO → 验证硬件
   步骤2: 任务里只 toggle 不 delay → 验证任务是否被调度
   步骤3: 加 vTaskDelay → 验证 tick 是否正常
*/

static void task_led_blink(void *arg)
{   
    gpio_bits_reset(GPIOA, GPIO_PINS_1 | GPIO_PINS_2);
    while (1) {
        gpio_bits_set(GPIOA, GPIO_PINS_1);
        gpio_bits_toggle(GPIOA, GPIO_PINS_0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_bits_reset(GPIOA, GPIO_PINS_0);
        gpio_bits_set(GPIOA, GPIO_PINS_1 | GPIO_PINS_2);
    }
}

int main(void)
{
    systick_clock_source_config(SYSTICK_CLOCK_SOURCE_AHBCLK_NODIV);
    gpio_init_led();

    /* 步骤1: 直接点亮全部 LED，证明 GPIO 正常 */
    gpio_bits_reset(GPIOA, GPIO_PINS_0 | GPIO_PINS_1 | GPIO_PINS_2);
    volatile uint32_t d;
    for(d = 0; d < 200000; d++);   /* ~250ms @8MHz */
    /* 步骤1: 全部熄灭 */
    gpio_bits_set(GPIOA, GPIO_PINS_0 | GPIO_PINS_1 | GPIO_PINS_2);
    for(d = 0; d < 200000; d++);

    /* 如果上面三步能看到 LED 先亮后灭，说明 GPIO 没问题 */
    /* 接下来进入 FreeRTOS */

    xTaskCreate(task_led_blink, "led", 256, NULL, 1, NULL);
    vTaskStartScheduler();

    /* 不应该到这里 */
    while (1);
}

void xPortSysTickHandler(void);
void vPortSVCHandler(void);
void xPortPendSVHandler(void);

void SysTick_Handler(void)  { xPortSysTickHandler(); }
void SVC_Handler(void)      { vPortSVCHandler(); }
void PendSV_Handler(void)   { xPortPendSVHandler(); }