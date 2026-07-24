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
extern volatile uint16_t adc1_ordinary_value;

extern volatile uint16_t uart_rx_len;
extern volatile uint8_t uart_rx_done;
extern uint8_t usart_rx_buf[];

extern can_rx_message_type can_rx_msg;
extern volatile uint8_t can_rx_done;
//Pure initializaion Area
void system_clock_config(void);
void dma_temp_config(void);
void pwm_init(void);
void adc_config(void);
void ertc_init(void);
void init_gpio_demo(void);
void init_usart_rv_dma_demo(void);
void systick_1ms_init(void);
void uart_print_init(uint32_t baudrate);
void at32_board_init(void);
void init_can1_demo(void);
//Utility Area
void simple_read(void);
void ertc_print_time(void);
void shift_pwn_mode(uint32_t pin);
void reset_usart_dma(void);
void can1_send(uint32_t id, uint8_t *data, uint8_t len);
#endif