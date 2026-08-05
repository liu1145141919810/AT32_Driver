#ifndef INIT_h
#define INIT_h
#include <stdint.h>

#define USART_RX_BUF_LEN  128
#define CAN_RX_BUF_LEN 64


/** @brief It also include some Hardware driver methods */
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
void initPrintDMA(char* buffer,int length);
//===================  Data Interface Area ====================== 
volatile uint16_t adc_read(int position);
volatile uint16_t uart_rx_len_read(void);
void uart_rx_len_write(uint16_t len);
uint8_t usart_rx_buf_read(int index);
void usart_rx_buf_write(int index, uint8_t value);
uint32_t arr_value_read(void);
#endif