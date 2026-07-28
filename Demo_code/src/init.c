#include <stdio.h>
#include "init.h"
#include "at32f423_tmr.h"

#define ADC_VREF                         (3.3)
#define ADC_TEMP_BASE                    (1.29)
#define ADC_TEMP_SLOPE                   (-0.00426)
#define MS_TICK (SystemCoreClock / 1000U)

#define PRINT_UART                       USART1
#define PRINT_UART_TX_GPIO_CRM_CLK       CRM_GPIOA_PERIPH_CLOCK
#define PRINT_UART_TX_PIN                GPIO_PINS_9
#define PRINT_UART_TX_GPIO               GPIOA
#define PRINT_UART_TX_PIN_SOURCE         GPIO_PINS_SOURCE9
#define PRINT_UART_CRM_CLK               CRM_USART1_PERIPH_CLOCK
#define PRINT_UART_TX_PIN_MUX_NUM        GPIO_MUX_7

#define USER_BUTTON_CRM_CLK              CRM_GPIOA_PERIPH_CLOCK
#define USER_BUTTON_PIN                  GPIO_PINS_0
#define USER_BUTTON_PORT                 GPIOA

uint8_t usart_rx_buf[USART_RX_BUF_LEN];
volatile uint16_t uart_rx_len = 0;
volatile uint8_t uart_rx_done = 0;

can_rx_message_type can_rx_msg;
volatile uint8_t can_rx_done = 0;
#include <unistd.h>  // _write 需要这个

#if defined (__GNUC__) && !defined (__clang__)
int _write(int fd, char *pbuffer, int size) {
    UNUSED(fd);
    //这里URAT默认用了URAT1
    for(int i = 0; i < size; i++) {
        while(usart_flag_get(PRINT_UART, USART_TDBE_FLAG) == RESET);
        usart_data_transmit(PRINT_UART, (uint16_t)(*pbuffer++));
        while(usart_flag_get(PRINT_UART, USART_TDC_FLAG) == RESET);
    }
    return size;
}
#endif
/** @brief It also include some Hardware driver methods */
//Pure initializaion Area
volatile uint16_t adc1_ordinary_value[2] = {0};

void system_clock_config(void)
{
    flash_psr_set(FLASH_WAIT_CYCLE_4);
    crm_periph_clock_enable(CRM_PWC_PERIPH_CLOCK, TRUE);
    pwc_ldo_output_voltage_set(PWC_LDO_OUTPUT_1V3);

    crm_clock_source_enable(CRM_CLOCK_SOURCE_HEXT, TRUE);
    while(crm_hext_stable_wait() == ERROR);

    /* PLL: HEXT 8MHz → 144MHz, APB1=/2 → 72MHz */
    crm_pll_config(CRM_PLL_SOURCE_HEXT, 72, 1, CRM_PLL_FR_2);
    crm_clock_source_enable(CRM_CLOCK_SOURCE_PLL, TRUE);
    while(crm_flag_get(CRM_PLL_STABLE_FLAG) != SET);

    crm_ahb_div_set(CRM_AHB_DIV_1);
    crm_apb2_div_set(CRM_APB2_DIV_1);
    crm_apb1_div_set(CRM_APB1_DIV_2);

    crm_auto_step_mode_enable(TRUE);
    crm_sysclk_switch(CRM_SCLK_PLL);
    while(crm_sysclk_switch_status_get() != CRM_SCLK_PLL);
    crm_auto_step_mode_enable(FALSE);

    system_core_clock_update();
}
void dma_temp_velo_config(void)
{
  dma_init_type dma_init_struct;
  crm_periph_clock_enable(CRM_DMA1_PERIPH_CLOCK, TRUE);

  dma_reset(DMA1_CHANNEL1);
  dma_default_para_init(&dma_init_struct);
  dma_init_struct.buffer_size = 2;
  dma_init_struct.direction = DMA_DIR_PERIPHERAL_TO_MEMORY;
  dma_init_struct.memory_base_addr = (uint32_t)&adc1_ordinary_value[0];
  dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_HALFWORD;
  dma_init_struct.memory_inc_enable = TRUE;
  dma_init_struct.peripheral_base_addr = (uint32_t)&(ADC1->odt);
  dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_HALFWORD;
  dma_init_struct.peripheral_inc_enable = FALSE;
  dma_init_struct.priority = DMA_PRIORITY_HIGH;
  dma_init_struct.loop_mode_enable = TRUE;
  dma_init(DMA1_CHANNEL1, &dma_init_struct);

  dmamux_enable(DMA1, TRUE);
  //配置DMAMUX通道1，选择ADC1作为DMA请求源
  dmamux_init(DMA1MUX_CHANNEL1, DMAMUX_DMAREQ_ID_ADC1);
  /* disable dma transfer complete interrupt */
  dma_interrupt_enable(DMA1_CHANNEL1, DMA_FDT_INT, FALSE);
  dma_channel_enable(DMA1_CHANNEL1, TRUE);
}

uint32_t arr_value;                     
volatile uint32_t systemticks = 0;  
void pwm_init(void) {
    tmr_output_config_type tmr_output_struct;
    // 只使能定时器时钟
    crm_periph_clock_enable(CRM_TMR2_PERIPH_CLOCK, TRUE);
    // 配置定时器参数
    crm_clocks_freq_type clock_freq;
    crm_clocks_freq_get(&clock_freq);  // 读取实际系统时钟 // 目标 PWM 频率 20kHz
    uint32_t target_freq = 20000;//选定大周期的频率为20000
    arr_value = (clock_freq.apb2_freq / target_freq);
    tmr_base_init(TMR2, arr_value-1, 0);
    tmr_cnt_dir_set(TMR2, TMR_COUNT_UP);
    // 不配置通道，不配置 GPIO ← 关键// 等进入 BRIGHT 状态时才配置
    tmr_counter_enable(TMR2, TRUE);
}
void adc_config(void)
{
  adc_common_config_type adc_common_struct;
  adc_base_config_type adc_base_struct;
  crm_periph_clock_enable(CRM_ADC1_PERIPH_CLOCK, TRUE);
  adc_reset();
  //nvic_irq_enable(ADC1_IRQn, 0, 0);
  crm_adc_clock_select(CRM_ADC_CLOCK_SOURCE_HCLK);

  adc_common_default_para_init(&adc_common_struct);

  /* config division,adcclk is division by hclk */
  adc_common_struct.div = ADC_HCLK_DIV_4;

  /* config inner temperature sensor and vintrv */
  adc_common_struct.tempervintrv_state = TRUE;

  adc_common_config(&adc_common_struct);

  adc_base_default_para_init(&adc_base_struct);

  adc_base_struct.sequence_mode = TRUE;
  adc_base_struct.repeat_mode = FALSE;
  adc_base_struct.data_align = ADC_RIGHT_ALIGNMENT;
  adc_base_struct.ordinary_channel_length = 2;
  adc_base_config(ADC1, &adc_base_struct);
  adc_resolution_set(ADC1, ADC_RESOLUTION_12B);

  /* config ordinary channel */
  adc_ordinary_channel_set(ADC1, ADC_CHANNEL_16, 1, ADC_SAMPLETIME_640_5);
  adc_ordinary_channel_set(ADC1, ADC_CHANNEL_17, 2, ADC_SAMPLETIME_640_5);

  /* config ordinary trigger source and trigger edge */
  adc_ordinary_conversion_trigger_set(ADC1, ADC_ORDINARY_TRIG_TMR1CH1, ADC_ORDINARY_TRIG_EDGE_NONE);

  /* config dma mode,it's not useful when common dma mode is use */
  adc_dma_mode_enable(ADC1, TRUE);

  /* config dma request repeat,it's not useful when common dma mode is use */
  adc_dma_request_repeat_enable(ADC1, TRUE);
  /* enable adc overflow interrupt */
  //adc_interrupt_enable(ADC1, ADC_OCCO_INT, TRUE);
  /* adc enable */
  adc_enable(ADC1, TRUE);
  while(adc_flag_get(ADC1, ADC_RDY_FLAG) == RESET);

  /* adc calibration */
  adc_calibration_init(ADC1);
  while(adc_calibration_init_status_get(ADC1));
  adc_calibration_start(ADC1);
  while(adc_calibration_status_get(ADC1));
}

void ertc_init(void)
{
    /* 1. 使能 PWC，允许后备域访问 */
    crm_periph_clock_enable(CRM_PWC_PERIPH_CLOCK, TRUE);
    pwc_battery_powered_domain_access(TRUE);
    /* 2. 检查是否已初始化 */
    if (ertc_bpr_data_read(ERTC_DT1) == 0x1234) {
        ertc_wait_update();
        return;
    }
    /* 3. 复位 ERTC 域 */
    crm_battery_powered_domain_reset(TRUE);
    crm_battery_powered_domain_reset(FALSE);

    
    /* 4. 使能 LICK 时钟源 (内部低速 RC，约 32kHz) */
    crm_clock_source_enable(CRM_CLOCK_SOURCE_LICK, TRUE);
    while (crm_flag_get(CRM_LICK_STABLE_FLAG) == RESET);
    crm_ertc_clock_select(CRM_ERTC_CLOCK_LICK);
    crm_ertc_clock_enable(TRUE);
    /* 5. 复位 ERTC 并设置分频 */
    ertc_reset();
    ertc_wait_update();
    ertc_divider_set(127, 255);  // 32768 / 128 / 256 ≈ 1 Hz
    /* 6. 设置时间 */
    ertc_hour_mode_set(ERTC_HOUR_MODE_24);
    ertc_date_set(26, 7, 17, 5);  // 2026-07-17, 周五
    ertc_time_set(0, 0, 0, ERTC_24H);
    /* 7. 标记已初始化 */
    ertc_bpr_data_write(ERTC_DT1, 0x1234);
}
void init_gpio_demo(){
    wait_for_power_stable();

    crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
    gpio_init_type gpio_init_struct;
    gpio_default_para_init(&gpio_init_struct);
    gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pins = GPIO_PINS_0 | GPIO_PINS_1 | GPIO_PINS_2;
    gpio_init(GPIOA, &gpio_init_struct);

}
void init_usart_rv_dma_demo(){
    gpio_init_type gpio_init_struct;
    crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_USART1_PERIPH_CLOCK, TRUE);

    /* 配置 PA10 为 USART1_RX (复用推挽) */
    gpio_default_para_init(&gpio_init_struct);
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_pins = GPIO_PINS_10;
    gpio_init_struct.gpio_pull = GPIO_PULL_UP;
    gpio_init(GPIOA, &gpio_init_struct);
    gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE10, GPIO_MUX_7);

    /* 使能 USART1 接收和发送 */
    usart_transmitter_enable(USART1, TRUE);
    usart_receiver_enable(USART1, TRUE);
    /* 使能 USART1 外设 */
    usart_enable(USART1, TRUE);
    usart_dma_receiver_enable(USART1, TRUE);

    //==============================
    /** @brief Following is the DMA configuration for USART1 */
    dma_init_type dma_init_struct;
    nvic_irq_enable(DMA1_Channel2_IRQn, 0, 0);

    dma_reset(DMA1_CHANNEL2);
    dma_default_para_init(&dma_init_struct);
    dma_init_struct.buffer_size = USART_RX_BUF_LEN;
    dma_init_struct.direction = DMA_DIR_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_base_addr = (uint32_t)usart_rx_buf;//Itself is indeed a pointer, just shift to uint32
    dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;//Every transimit one byte
    dma_init_struct.memory_inc_enable = TRUE;
    dma_init_struct.peripheral_base_addr = (uint32_t)&(USART1->dt);
    dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
    dma_init_struct.peripheral_inc_enable = FALSE;
    dma_init_struct.priority = DMA_PRIORITY_HIGH;
    dma_init_struct.loop_mode_enable = TRUE;
    dma_init(DMA1_CHANNEL2, &dma_init_struct);

    dmamux_enable(DMA1, TRUE);
    //配置DMAMUX通道2，选择USART1_RX作为DMA请求源
    dmamux_init(DMA1MUX_CHANNEL2, DMAMUX_DMAREQ_ID_USART1_RX);
    dma_channel_enable(DMA1_CHANNEL2, TRUE);

    // 用 USART IDLE 中断检测一帧结束，而非 DMA FDT 中断
    usart_interrupt_enable(USART1, USART_IDLE_INT, TRUE);
    nvic_irq_enable(USART1_IRQn, 0, 0);
}
void systick_1ms_init(void) {
    systick_clock_source_config(SYSTICK_CLOCK_SOURCE_AHBCLK_NODIV);
    SysTick->LOAD  = (uint32_t)(MS_TICK - 1UL);
    NVIC_SetPriority(SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);
    SysTick->VAL   = 0UL;
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}
void uart_print_init(uint32_t baudrate)
{
  gpio_init_type gpio_init_struct;

#if defined (__GNUC__) && !defined (__clang__)
  setvbuf(stdout, NULL, _IONBF, 0);
#endif

  /* enable the uart and gpio clock */
  crm_periph_clock_enable(PRINT_UART_CRM_CLK, TRUE);
  crm_periph_clock_enable(PRINT_UART_TX_GPIO_CRM_CLK, TRUE);

  gpio_default_para_init(&gpio_init_struct);

  /* configure the uart tx pin */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = PRINT_UART_TX_PIN;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(PRINT_UART_TX_GPIO, &gpio_init_struct);

  gpio_pin_mux_config(PRINT_UART_TX_GPIO, PRINT_UART_TX_PIN_SOURCE, PRINT_UART_TX_PIN_MUX_NUM);

  /* configure uart param */
  usart_init(PRINT_UART, baudrate, USART_DATA_8BITS, USART_STOP_1_BIT);
  usart_transmitter_enable(PRINT_UART, TRUE);
  usart_enable(PRINT_UART, TRUE);
}

void at32_board_init()
{
  /* initialize delay function */
  systick_clock_source_config(SYSTICK_CLOCK_SOURCE_AHBCLK_NODIV);
  fac_us = system_core_clock / (1000000U);
  fac_ms = fac_us * (1000U);
  /* configure button in at_start board */
  gpio_init_type gpio_init_struct;

  /* enable the button clock */
  crm_periph_clock_enable(USER_BUTTON_CRM_CLK, TRUE);

  /* set default parameter */
  gpio_default_para_init(&gpio_init_struct);

  /* configure button pin as input with pull-up/pull-down */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
  gpio_init_struct.gpio_pins = USER_BUTTON_PIN;
  gpio_init_struct.gpio_pull = GPIO_PULL_DOWN;
  gpio_init(USER_BUTTON_PORT, &gpio_init_struct);
}
void init_can1_demo(void)
{
    gpio_init_type gpio_init_struct;
    can_base_type can_base_struct;
    can_baudrate_type can_baudrate_struct;
    can_filter_init_type can_filter_init_struct;

    /* GPIO: PA11=RX, PA12=TX, MUX_9 (原理图 THA1040) */
    crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
    gpio_default_para_init(&gpio_init_struct);
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_pins = GPIO_PINS_11 | GPIO_PINS_12;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;//无内部电阻干扰
    gpio_init(GPIOA, &gpio_init_struct);
    gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE11, GPIO_MUX_9);
    gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE12, GPIO_MUX_9);

    /* CAN1 时钟 */
    crm_periph_clock_enable(CRM_CAN1_PERIPH_CLOCK, TRUE);

    /* CAN 基础配置 */
    can_default_para_init(&can_base_struct);
    can_base_struct.mode_selection = CAN_MODE_COMMUNICATE;
    can_base_struct.aebo_enable = TRUE;   // 自动退出 bus-off
    can_base_struct.aed_enable = TRUE;    // 自动退出 doze
    can_base_init(CAN1, &can_base_struct);

    /* 波特率 (APB1=72MHz): 72M / (9 × (3+12+3)) = 500Kbps */
    can_baudrate_struct.baudrate_div = 9;
    can_baudrate_struct.rsaw_size = CAN_RSAW_2TQ;//偏差调整
    can_baudrate_struct.bts1_size = CAN_BTS1_12TQ;
    can_baudrate_struct.bts2_size = CAN_BTS2_3TQ;
    can_baudrate_set(CAN1, &can_baudrate_struct);

    /* 滤波器: 全接收(不过滤) */
    can_filter_init_struct.filter_activate_enable = TRUE;
    can_filter_init_struct.filter_mode = CAN_FILTER_MODE_ID_MASK;
    can_filter_init_struct.filter_fifo = CAN_FILTER_FIFO0;
    can_filter_init_struct.filter_number = 0;
    can_filter_init_struct.filter_bit = CAN_FILTER_32BIT;
    can_filter_init_struct.filter_id_high = 0;//所有ID都接收
    can_filter_init_struct.filter_id_low = 0;
    can_filter_init_struct.filter_mask_high = 0;  // mask=0 → 全接收
    can_filter_init_struct.filter_mask_low = 0;
    can_filter_init(CAN1, &can_filter_init_struct);

    /* 中断 */
    nvic_irq_enable(CAN1_RX0_IRQn, 0x00, 0x00);
    can_interrupt_enable(CAN1, CAN_RF0MIEN_INT, TRUE);
}

//Utility Area
void simple_read(){
    adc_ordinary_software_trigger_enable(ADC1, TRUE);
    while(dma_flag_get(DMA1_FDT1_FLAG) == RESET);
    dma_flag_clear(DMA1_FDT1_FLAG);
    printf("internal_temperature = %.2f deg C\r\n",
        (ADC_TEMP_BASE - (double)adc1_ordinary_value[0] * ADC_VREF / 4095) / ADC_TEMP_SLOPE + 25);
    printf("internal_vref = %.3f V\r\n",
        ((double)1.2 * 4095) / adc1_ordinary_value[1]);
}
/* 单次输出当前时间 */
void ertc_print_time(void)
{
    ertc_time_type t;
    ertc_calendar_get(&t);
    printf("RTC: 20%02d-%02d-%02d %02d:%02d:%02d\r\n",
           t.year, t.month, t.day, t.hour, t.min, t.sec);
}
void shift_pwn_mode(uint32_t pin){
    //Only GPIOA pins 0, 1, 2 are supported for this demo
    gpio_init_type gpio_init_struct;
    gpio_default_para_init(&gpio_init_struct);
    tmr_output_config_type tmr_output_struct;
    gpio_init_struct.gpio_pins = pin;
    
    //pin:硬件引脚位
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init(GPIOA, &gpio_init_struct);
    
    //复用引脚位置
    gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE0, GPIO_MUX_1);  // TMR2_CH1
    gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE1, GPIO_MUX_1);  // TMR2_CH2
    gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE2, GPIO_MUX_1);  // TMR2_CH3
    
    // 配置 PWM 通道
    tmr_output_default_para_init(&tmr_output_struct);
    tmr_output_struct.oc_mode = TMR_OUTPUT_CONTROL_PWM_MODE_A;// 设置为 PWM 模式 A,高低判断
    tmr_output_struct.oc_output_state = TRUE;//内部状态直接输出到波形
    tmr_output_struct.oc_polarity = TMR_OUTPUT_ACTIVE_HIGH;//有效电平极性
    
    tmr_output_channel_config(TMR2, TMR_SELECT_CHANNEL_1, &tmr_output_struct);
    tmr_output_channel_config(TMR2, TMR_SELECT_CHANNEL_2, &tmr_output_struct);
    tmr_output_channel_config(TMR2, TMR_SELECT_CHANNEL_3, &tmr_output_struct);
}
void reset_usart_dma(void){
    dma_channel_enable(DMA1_CHANNEL2, FALSE);
    dma_data_number_set(DMA1_CHANNEL2, USART_RX_BUF_LEN);
    dma_channel_enable(DMA1_CHANNEL2, TRUE);
}
void can1_send(uint32_t id, uint8_t *data, uint8_t len)
{
    can_tx_message_type tx_msg;
    tx_msg.standard_id = id;
    tx_msg.extended_id = 0;//更多空间的id
    tx_msg.id_type = CAN_ID_STANDARD;
    tx_msg.frame_type = CAN_TFT_DATA;
    tx_msg.dlc = len;
    for (int i = 0; i < len && i < 8; i++)
        tx_msg.data[i] = data[i];
    can_message_transmit(CAN1, &tx_msg);
}
//Interupt Area
/*
void SysTick_Handler(void) {//此函数中断时自动给录入触发
    systemticks++;
}
*/

void USART1_IRQHandler(void) {//Idle Detect Interrupt
    if(usart_flag_get(USART1, USART_IDLEF_FLAG) != RESET)
    {
        usart_data_receive(USART1);  // 读 DR 清除 IDLE 标志，除此外无用
        uart_rx_len = USART_RX_BUF_LEN - dma_data_number_get(DMA1_CHANNEL2);
        uart_rx_done = 1;
    }
}//CAN会自己管理边界，不需要重置
void CAN1_RX0_IRQHandler(void)
{//触发通信逻辑，这里是简单地提示存取，可以复杂化
    if (can_interrupt_flag_get(CAN1, CAN_RF0MN_FLAG) != RESET)
    {
        can_message_receive(CAN1, CAN_RX_FIFO0, &can_rx_msg);
        can_rx_done = 1;
    }
}