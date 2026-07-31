#include "utility.h"
#include "at32f423_can.h"
#include "init.h"
#include <stdarg.h>
#define ADC_VREF                         (3.3)
#define ADC_TEMP_BASE                    (1.29)
#define ADC_TEMP_SLOPE                   (-0.00426)
#define PRINT_UART                       USART1
void simple_read(){
    adc_ordinary_software_trigger_enable(ADC1, TRUE);
    while(dma_flag_get(DMA1_FDT1_FLAG) == RESET);
    dma_flag_clear(DMA1_FDT1_FLAG);
    demoPrint("internal_temperature = %.2f deg C\r\n",
        (ADC_TEMP_BASE - (double)adc_read(0) * ADC_VREF / 4095) / ADC_TEMP_SLOPE + 25);
    demoPrint("internal_vref = %.3f V\r\n",
        ((double)1.2 * 4095) / (double)adc_read(1));
    }
/* Single output of current time */
void ertc_print_time(void)
{   //Here we don't spare 0 for the relevant data
    ertc_time_type t;
    ertc_calendar_get(&t);
    //printf("RTC: 20%02d-%02d-%02d %02d:%02d:%02d\r\n",
    //       t.year, t.month, t.day, t.hour, t.min, t.sec);
    demoPrint("RTC: %d-%d-%d %d:%d:%d\r\n",
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
void can1_send(uint32_t id, uint8_t *data, uint8_t len)
{
    can_tx_message_type tx_msg;
    tx_msg.standard_id = id;
    tx_msg.extended_id = 0;//Id for the extended frame, not used in this demo
    tx_msg.id_type = CAN_ID_STANDARD;
    tx_msg.frame_type = CAN_TFT_DATA;
    tx_msg.dlc = len;
    for (int i = 0; i < len && i < 8; i++)
        tx_msg.data[i] = data[i];
    can_message_transmit(CAN1, &tx_msg);
}
static void sendUSART(char c){
    while(usart_flag_get(PRINT_UART, USART_TDBE_FLAG) == RESET);
    usart_data_transmit(PRINT_UART, (uint16_t)(c));
    while(usart_flag_get(PRINT_UART, USART_TDC_FLAG) == RESET);
}
static void printNumI(int num){
    char buffer[16];
    int p=0;
    if(num<0)
    {
        sendUSART('-');
        num=-num;
    }
    if(num==0)
    {sendUSART('0');}
    else
    {
        while(num>0){
            buffer[p++]=num%10+'0';
            num/=10;
            if(p>=16) break;
        }
        for(int i=p-1;i>=0;i--)
        {sendUSART(buffer[i]);}
    }
}
//Accuracy can not be higher than 9 decimal places, otherwise it will be rounded to 9 decimal places
static void printNumF(double num,int decimalPlaces)
{
    char buffer[16];
    int p=0;
    if(num<0)
    {
        sendUSART('-');
        num=-num;
    }
    if(decimalPlaces>0){
        double round_base=1;
        for(int i=0;i<decimalPlaces;i++) round_base *=10;
        num=num+0.5/round_base;
    }
    int integer=(int)num;
    double fraction=num-integer;//This part must have a pre-dealing for rounding
    // 处理整数
    if(integer==0)
    {
        sendUSART('0');
    }
    else
    {
        while(integer>0){
            buffer[p++]=integer%10+'0';
            integer/=10;
            if(p>=16) break;
        }
        for(int i=p-1;i>=0;i--)
        {sendUSART(buffer[i]);}
    }
    //处理小数
    if(decimalPlaces>0)
    {
        sendUSART('.');
        while(decimalPlaces--)
        {
            fraction*=10;
            int digit=(int)fraction;
            sendUSART(digit+'0');
            fraction-=digit;
        }
    }
}
void demoPrint(const char* str,...){
    va_list args;
    va_start(args, str);
    while(*str){
        int x;
        if(*str=='%'){
            str++;
            if(*str=='d'){//Integer Dealing
                int val=va_arg(args, int);
                printNumI(val);
            }
            else if(*str == 's'){//Char String Dealing
                char* s = va_arg(args, char*);
                while(*s){
                    sendUSART(*s);
                    s++;
                }
            }
            else if(*str == '.'){
                str+=1;
                double val = va_arg(args, double);
                int store=*str - '0';//Get the number of decimal places to display, no more than 9
                str+=1;
                printNumF(val,store);
            }
        }
        else{
            int y;
            sendUSART(*str);
        }
        str++;
    }
    va_end(args);
}