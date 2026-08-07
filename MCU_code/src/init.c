#include <stdio.h>

#include "at32f423_tmr.h"

#include "init.h"
#include "public_define.h"
#define ADC_VREF                         (3.3)
#define ADC_TEMP_BASE                    (1.29)
#define ADC_TEMP_SLOPE                   (-0.00426)

uint8_t usart_rx_buf[USART_RX_BUF_LEN];
volatile uint16_t uart_rx_len = 0;
/*DMA
*Channel 1: for Tempature and Vref Monitor
*Channel 2: for USART1 RX
*Channel 3: for USART1 TX
*/

#include <unistd.h>  // Required for _write function redirection

/** @brief This file integrates multiple hardware peripheral driver initialization functions */
// Pure Hardware Initialization Section
volatile uint16_t adc1_ordinary_value[2] = {0};

void system_clock_config(void)
{   // Configure main system clock frequency to 144MHz
    flash_psr_set(FLASH_WAIT_CYCLE_4);
    crm_periph_clock_enable(CRM_PWC_PERIPH_CLOCK, TRUE);
    pwc_ldo_output_voltage_set(PWC_LDO_OUTPUT_1V3);

    crm_clock_source_enable(CRM_CLOCK_SOURCE_HEXT, TRUE);
    while(crm_hext_stable_wait() == ERROR);

    /* PLL configuration: HEXT 8MHz input → final 144MHz system clock, APB1 prescaler = 2 → APB1 bus clock 72MHz */
    crm_pll_config(CRM_PLL_SOURCE_HEXT, 72, 1, CRM_PLL_FR_2);
    crm_clock_source_enable(CRM_CLOCK_SOURCE_PLL, TRUE);
    while(crm_flag_get(CRM_PLL_STABLE_FLAG) != SET);

    crm_ahb_div_set(CRM_AHB_DIV_1);
    crm_apb2_div_set(CRM_APB2_DIV_1);
    crm_apb1_div_set(CRM_APB1_DIV_2);

    crm_auto_step_mode_enable(TRUE);
    crm_sysclk_switch(CRM_SCLK_PLL);// Switch main system clock source to PLL output
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
  // Configure DMAMUX channel 1, select ADC1 as DMA trigger request source
  dmamux_init(DMA1MUX_CHANNEL1, DMAMUX_DMAREQ_ID_ADC1);
  /* Disable DMA transfer complete interrupt */
  dma_interrupt_enable(DMA1_CHANNEL1, DMA_FDT_INT, FALSE);
  dma_channel_enable(DMA1_CHANNEL1, TRUE);
}

uint32_t arr_value;
volatile uint32_t systemticks = 0;
void pwm_init(void) {
    tmr_output_config_type tmr_output_struct;
    // Enable clock only for Timer peripheral
    crm_periph_clock_enable(CRM_TMR2_PERIPH_CLOCK, TRUE);
    // Read real bus clock frequency for calculation
    crm_clocks_freq_type clock_freq;
    crm_clocks_freq_get(&clock_freq);
    uint32_t target_freq = 20000;// Target PWM carrier frequency set to 20kHz
    arr_value = (clock_freq.apb2_freq / target_freq);
    tmr_base_init(TMR2, arr_value-1, 0);
    tmr_cnt_dir_set(TMR2, TMR_COUNT_UP);
    // Timer channels and corresponding GPIO pins are not configured here (critical note)
    // Pin multiplexing will be configured after entering BRIGHT state
    tmr_counter_enable(TMR2, TRUE);
}
void adc_config(void)
{
  adc_common_config_type adc_common_struct;
  adc_base_config_type adc_base_struct;
  crm_periph_clock_enable(CRM_ADC1_PERIPH_CLOCK, TRUE);
  adc_reset();
  crm_adc_clock_select(CRM_ADC_CLOCK_SOURCE_HCLK);

  adc_common_default_para_init(&adc_common_struct);

  /* Configure clock division: ADC clock derived from HCLK with divider */
  adc_common_struct.div = ADC_HCLK_DIV_4;

  /* Enable internal temperature sensor and internal reference voltage channel */
  adc_common_struct.tempervintrv_state = TRUE;

  adc_common_config(&adc_common_struct);

  adc_base_default_para_init(&adc_base_struct);

  adc_base_struct.sequence_mode = TRUE;
  adc_base_struct.repeat_mode = FALSE;
  adc_base_struct.data_align = ADC_RIGHT_ALIGNMENT;
  adc_base_struct.ordinary_channel_length = 2;
  adc_base_config(ADC1, &adc_base_struct);
  adc_resolution_set(ADC1, ADC_RESOLUTION_12B);

  /* Assign ADC regular conversion channels and sampling sequence order */
  adc_ordinary_channel_set(ADC1, ADC_CHANNEL_16, 1, ADC_SAMPLETIME_640_5);
  adc_ordinary_channel_set(ADC1, ADC_CHANNEL_17, 2, ADC_SAMPLETIME_640_5);

  /* Configure trigger source and edge for regular channel conversion */
  adc_ordinary_conversion_trigger_set(ADC1, ADC_ORDINARY_TRIG_TMR1CH1, ADC_ORDINARY_TRIG_EDGE_NONE);

  /* Enable ADC DMA mode; this setting is redundant when shared DMA mode is active */
  adc_dma_mode_enable(ADC1, TRUE);

  /* Enable DMA repeat request; redundant under shared DMA mode */
  adc_dma_request_repeat_enable(ADC1, TRUE);
  /* Enable ADC overflow error interrupt */
  //adc_interrupt_enable(ADC1, ADC_OCCO_INT, TRUE);
  /* Power up ADC module */
  adc_enable(ADC1, TRUE);
  while(adc_flag_get(ADC1, ADC_RDY_FLAG) == RESET);

  /* Execute ADC self-calibration procedure */
  adc_calibration_init(ADC1);
  while(adc_calibration_init_status_get(ADC1));
  adc_calibration_start(ADC1);
  while(adc_calibration_status_get(ADC1));
}
void config_time(int year, int month, int day,int weekday, int hour, int minute, int second) {
    ertc_wait_update();
    ertc_hour_mode_set(ERTC_HOUR_MODE_24);
    ertc_date_set(year, month, day, weekday);
    ertc_time_set(hour, minute, second, ERTC_24H);
    ertc_bpr_data_write(ERTC_DT1, 0x1234);  // Write signature flag to indicate successful initialization
}
void ertc_init(void)
{
    /* 1. Enable PWC clock to grant access to backup power domain */
    crm_periph_clock_enable(CRM_PWC_PERIPH_CLOCK, TRUE);
    pwc_battery_powered_domain_access(TRUE);
    /* 2. Check if ERTC module has been initialized before */
    if (ertc_bpr_data_read(ERTC_DT1) == 0x1234) {
        ertc_wait_update();
        return;
    }
    /* 3. Perform full reset on backup power domain */
    crm_battery_powered_domain_reset(TRUE);
    crm_battery_powered_domain_reset(FALSE);


    /* 4. Enable LICK clock source (internal low-speed RC oscillator, nominal 32kHz) */
    crm_clock_source_enable(CRM_CLOCK_SOURCE_LICK, TRUE);
    while (crm_flag_get(CRM_LICK_STABLE_FLAG) == RESET);
    crm_ertc_clock_select(CRM_ERTC_CLOCK_LICK);
    crm_ertc_clock_enable(TRUE);
    /* 5. Reset ERTC peripheral and set clock prescaler */
    ertc_reset();
    ertc_wait_update();
    ertc_divider_set(127, 255);  // 32768 / 128 / 256 ≈ 1Hz real-time clock tick
    /* 6. Configure calendar date and time */
    //config_time(26, 7, 17, 5, 0, 0, 0);  // Set initial time to 2026-07-17 00:00:00
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
void init_usart_rv_dma_demo(){//USART1 RX DMA receiving initialization
    gpio_init_type gpio_init_struct;
    crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_USART1_PERIPH_CLOCK, TRUE);

    /* Configure PA10 as USART1 RX pin (alternate function push-pull input) */
    gpio_default_para_init(&gpio_init_struct);
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_pins = GPIO_PINS_10;
    gpio_init_struct.gpio_pull = GPIO_PULL_UP;
    gpio_init(GPIOA, &gpio_init_struct);
    gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE10, GPIO_MUX_7);

    /* Activate USART1 transmitter and receiver */
    usart_transmitter_enable(USART1, TRUE);
    usart_receiver_enable(USART1, TRUE);
    /* Enable USART1 peripheral module */
    usart_enable(USART1, TRUE);
    usart_dma_receiver_enable(USART1, TRUE);

    //==============================
    /** @brief DMA peripheral configuration dedicated to USART1 receive */
    dma_init_type dma_init_struct;

    dma_reset(DMA1_CHANNEL2);
    dma_default_para_init(&dma_init_struct);
    dma_init_struct.buffer_size = USART_RX_BUF_LEN;
    dma_init_struct.direction = DMA_DIR_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_base_addr = (uint32_t)usart_rx_buf;// Array name equals pointer address, cast to uint32 type
    dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;// Single byte transferred per DMA transaction
    dma_init_struct.memory_inc_enable = TRUE;
    dma_init_struct.peripheral_base_addr = (uint32_t)&(USART1->dt);
    dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
    dma_init_struct.peripheral_inc_enable = FALSE;
    dma_init_struct.priority = DMA_PRIORITY_HIGH;
    dma_init_struct.loop_mode_enable = TRUE;
    dma_init(DMA1_CHANNEL2, &dma_init_struct);

    dmamux_enable(DMA1, TRUE);
    // Configure DMAMUX channel 2, route USART1_RX request to DMA
    dmamux_init(DMA1MUX_CHANNEL2, DMAMUX_DMAREQ_ID_USART1_RX);
    dma_channel_enable(DMA1_CHANNEL2, TRUE);

    // Frame termination detected by USART IDLE interrupt instead of DMA transfer complete interrupt
    usart_interrupt_enable(USART1, USART_IDLE_INT, TRUE);
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

  /* Turn on clock power for target UART and corresponding TX GPIO */
  crm_periph_clock_enable(PRINT_UART_CRM_CLK, TRUE);
  crm_periph_clock_enable(PRINT_UART_TX_GPIO_CRM_CLK, TRUE);

  gpio_default_para_init(&gpio_init_struct);

  /* Configure UART transmit pin electrical parameters */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = PRINT_UART_TX_PIN;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(PRINT_UART_TX_GPIO, &gpio_init_struct);

  gpio_pin_mux_config(PRINT_UART_TX_GPIO, PRINT_UART_TX_PIN_SOURCE, PRINT_UART_TX_PIN_MUX_NUM);

  /* Initialize UART communication parameters */
  usart_init(PRINT_UART, baudrate, USART_DATA_8BITS, USART_STOP_1_BIT);
  usart_transmitter_enable(PRINT_UART, TRUE);
  usart_enable(PRINT_UART, TRUE);
  //Must take the following configure to allow it use DMA
  usart_dma_transmitter_enable(PRINT_UART, TRUE);
}

void at32_board_init()
{
  /* Initialize core delay timer base clock */
  systick_clock_source_config(SYSTICK_CLOCK_SOURCE_AHBCLK_NODIV);
  /* Configure user button hardware on AT32 demo board */
  gpio_init_type gpio_init_struct;

  /* Enable clock supply for button GPIO port */
  crm_periph_clock_enable(USER_BUTTON_CRM_CLK, TRUE);

  /* Load default GPIO configuration values */
  gpio_default_para_init(&gpio_init_struct);

  /* Set user button pin as digital input with pull-down resistor */
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

    /* GPIO assignment: PA11 = CAN RX, PA12 = CAN TX, multiplex function MUX_9 (matches THA1040 hardware schematic) */
    crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
    gpio_default_para_init(&gpio_init_struct);
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_pins = GPIO_PINS_11 | GPIO_PINS_12;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;// No internal pull resistor to avoid signal interference
    gpio_init(GPIOA, &gpio_init_struct);
    gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE11, GPIO_MUX_9);
    gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE12, GPIO_MUX_9);

    /* Enable CAN1 peripheral clock */
    crm_periph_clock_enable(CRM_CAN1_PERIPH_CLOCK, TRUE);

    /* CAN basic working mode configuration */
    can_default_para_init(&can_base_struct);
    can_base_struct.mode_selection = CAN_MODE_COMMUNICATE;
    can_base_struct.aebo_enable = TRUE;   // Auto recover from bus-off state
    can_base_struct.aed_enable = TRUE;    // Auto wakeup from sleep mode
    can_base_init(CAN1, &can_base_struct);

    /* Baud rate calculation (APB1 bus clock = 72MHz): 72M / (9 × (3+12+3)) = 500kbps */
    can_baudrate_struct.baudrate_div = 9;
    can_baudrate_struct.rsaw_size = CAN_RSAW_2TQ;// Resynchronization jump width setting
    can_baudrate_struct.bts1_size = CAN_BTS1_12TQ;
    can_baudrate_struct.bts2_size = CAN_BTS2_3TQ;
    can_baudrate_set(CAN1, &can_baudrate_struct);

    /* CAN filter configuration: receive all CAN IDs without hardware filtering */
    can_filter_init_struct.filter_activate_enable = TRUE;
    can_filter_init_struct.filter_mode = CAN_FILTER_MODE_ID_MASK;
    can_filter_init_struct.filter_fifo = CAN_FILTER_FIFO0;
    can_filter_init_struct.filter_number = 0;
    can_filter_init_struct.filter_bit = CAN_FILTER_32BIT;
    can_filter_init_struct.filter_id_high = 0;// Accept all arbitration IDs
    can_filter_init_struct.filter_id_low = 0;
    can_filter_init_struct.filter_mask_high = 0;  // Mask register all zero = all ID bits ignored
    can_filter_init_struct.filter_mask_low = 0;
    can_filter_init(CAN1, &can_filter_init_struct);

    /* Configure CAN receive interrupt priority */
    can_interrupt_enable(CAN1, CAN_RF0MIEN_INT, TRUE);
}

//General Utility Function Section
void reset_usart_dma(void){
    dma_channel_enable(DMA1_CHANNEL2, FALSE);
    dma_data_number_set(DMA1_CHANNEL2, USART_RX_BUF_LEN);
    dma_channel_enable(DMA1_CHANNEL2, TRUE);
}
void initPrintDMA(char* buffer,int length){
    dma_init_type dma_init_struct;
    dma_reset(DMA1_CHANNEL3);
    dma_default_para_init(&dma_init_struct);
    dma_init_struct.buffer_size = length;
    dma_init_struct.direction = DMA_DIR_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_base_addr = (uint32_t)buffer;
    dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
    dma_init_struct.memory_inc_enable = TRUE;
    dma_init_struct.peripheral_base_addr = (uint32_t)&USART1->dt;
    dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
    dma_init_struct.peripheral_inc_enable = FALSE;
    dma_init_struct.priority = DMA_PRIORITY_HIGH;
    dma_init_struct.loop_mode_enable = FALSE;
    dma_init(DMA1_CHANNEL3, &dma_init_struct);

    dmamux_enable(DMA1, TRUE);
    dmamux_init(DMA1MUX_CHANNEL3, DMAMUX_DMAREQ_ID_USART1_TX);// Configure DMAMUX channel 3, route USART1_TX request to DMA
    
    dma_interrupt_enable(DMA1_CHANNEL3, DMA_FDT_INT, TRUE);//Enable DMA interrupt for transfer complete
    if(length > 0)
        dma_channel_enable(DMA1_CHANNEL3, TRUE);

}

//=============== Public Data Read/Write Interface ======================
volatile uint16_t adc_read(int position){
    return adc1_ordinary_value[position];
}
volatile uint16_t uart_rx_len_read(void){
    return uart_rx_len;
}
void uart_rx_len_write(uint16_t len){
    uart_rx_len = len;
}
uint8_t usart_rx_buf_read(int index){
    return usart_rx_buf[index];
}
void usart_rx_buf_write(int index, uint8_t value){
    usart_rx_buf[index] = value;
}
uint32_t arr_value_read(void){
    return arr_value;
}