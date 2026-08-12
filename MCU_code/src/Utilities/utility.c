#include "utility.h"
#include "at32f423_can.h"
#include "init.h"
#include "Msg_Protocol.h"
#define ADC_VREF                         (3.3)
#define ADC_TEMP_BASE                    (1.29)
#define ADC_TEMP_SLOPE                   (-0.00426)
#define PRINT_UART                       USART1
void simple_read(CommandType print_state){
    adc_ordinary_software_trigger_enable(ADC1, TRUE);
    //This place could also be optimized
    while(dma_flag_get(DMA1_FDT1_FLAG) == RESET);
    dma_flag_clear(DMA1_FDT1_FLAG);
    msgPrint(print_state,"internal_temperature = %.2f deg C",
        (ADC_TEMP_BASE - (double)adc_read(0) * ADC_VREF / 4095) / ADC_TEMP_SLOPE + 25);
    msgPrint(print_state,"internal_vref = %.3f V",
        ((double)1.2 * 4095) / (double)adc_read(1));
    }
/* Single output of current time */
void ertc_print_time(CommandType print_state)
{   //Here we don't spare 0 for the relevant data
    ertc_time_type t;
    ertc_calendar_get(&t);
    //printf("RTC: 20%02d-%02d-%02d %02d:%02d:%02d\r\n",
    //       t.year, t.month, t.day, t.hour, t.min, t.sec);
    msgPrint(print_state,"RTC: %d-%d-%d %d:%d:%d",
       (2000+t.year), t.month, t.day, t.hour, t.min, t.sec);
}
void shift_pwn_mode(uint32_t pin){
    //Only GPIOA pins 0, 1, 2 are supported for this demo
    gpio_init_type gpio_init_struct;
    gpio_default_para_init(&gpio_init_struct);
    tmr_output_config_type tmr_output_struct;
    gpio_init_struct.gpio_pins = pin;
    
    //Hardware pin bit
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init(GPIOA, &gpio_init_struct);
    
    // Configure the pin's alternate function to TMR2
    gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE0, GPIO_MUX_1);  // TMR2_CH1
    gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE1, GPIO_MUX_1);  // TMR2_CH2
    gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE2, GPIO_MUX_1);  // TMR2_CH3
    
    // Configure TMR2 for PWM output
    tmr_output_default_para_init(&tmr_output_struct);
    tmr_output_struct.oc_mode = TMR_OUTPUT_CONTROL_PWM_MODE_A;// Setting the output channel to PWM mode A
    tmr_output_struct.oc_output_state = TRUE;//Internal output directly controls the output of the corresponding channel.
    tmr_output_struct.oc_polarity = TMR_OUTPUT_ACTIVE_HIGH;//Voltage high level is active
    
    tmr_output_channel_config(TMR2, TMR_SELECT_CHANNEL_1, &tmr_output_struct);
    tmr_output_channel_config(TMR2, TMR_SELECT_CHANNEL_2, &tmr_output_struct);
    tmr_output_channel_config(TMR2, TMR_SELECT_CHANNEL_3, &tmr_output_struct);
}