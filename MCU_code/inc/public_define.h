#ifndef PUBLIC_DEFINE_H
#define PUBLIC_DEFINE_H

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

//Calibrate
#define CALIBRATE_MONITOR_DELAY 100

//Command
#define CMD_BUF_SIZE 32

#endif