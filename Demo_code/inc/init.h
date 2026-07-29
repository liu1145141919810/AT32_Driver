#ifndef INIT_h
#define INIT_h
#include <stdint.h>
#include "at32f423_can.h"

#define USART_RX_BUF_LEN  128
#define CAN_RX_BUF_LEN 64

static volatile uint32_t fac_us;
static volatile uint32_t fac_ms;
/** @brief It also include some Hardware driver methods */
extern uint32_t arr_value;//used for pwm
extern volatile uint32_t systemticks;

extern volatile uint16_t uart_rx_len;
extern uint8_t usart_rx_buf[];

extern can_rx_message_type can_rx_msg;
//Pure initializaion Area
void system_clock_config(void);
void dma_temp_velo_config(void);
void pwm_init(void);
void adc_config(void);
void ertc_init(void);
void init_gpio_demo(void);
void init_usart_rv_dma_demo(void);
void systick_1ms_init(void);
void uart_print_init(uint32_t baudrate);
void at32_board_init(void);
void init_can1_demo(void);
void reset_usart_dma(void);
volatile uint16_t* adc_read(int position);
#endif