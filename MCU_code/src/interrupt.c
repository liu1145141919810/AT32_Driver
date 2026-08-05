#include "interrupt.h"
#include "at32f423_usart.h"
//========Priority Setting=================
void nvic_priority_config(void){
    nvic_irq_enable(ADC1_IRQn,
    2,
    0);
    nvic_irq_enable(DMA1_Channel2_IRQn,
    3,
    0);
    nvic_irq_enable(USART1_IRQn,
    3,
    0);
    nvic_irq_enable(CAN1_RX0_IRQn,
    2,
    0);
    nvic_irq_enable(DMA1_Channel3_IRQn,
    4,
    0);
}