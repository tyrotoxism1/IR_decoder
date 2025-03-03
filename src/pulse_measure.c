#include "pulse_measure.h"
#include "stm32f4xx.h"
#include "uart.h"
#include "printf.h"


struct pulse_measure_t{
    uint8_t capture_complete;
};

void TIM2_IRQHandler(void){
    // If TIM2 channel1 interrupt occured 
    if(TIM2->SR & TIM_SR_CC1IF){
        GPIOA->ODR ^= GPIO_ODR_OD4; 
        //clear status register to acknowledge interrupt has fired
        TIM2->SR &= ~(TIM_SR_CC1IF);
    }
}

// Defualt port used for input capture will be PA0
// For TIM2_CH1, must be set to AF1
static void _pulse_measure_gpio_init(void){
    // Enable GPIO port A 
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; 
    // Set PA0 to AF mode
    GPIOA->MODER |= GPIO_MODER_MODE0_1;
    // Use PA4 as debug LED, set it to be output mode 
    GPIOA->MODER |= GPIO_MODER_MODE4_0;
    // clear and set AF1 for PA0
    GPIOA->AFR[0] &= ~(GPIO_AFRL_AFRL0);
    GPIOA->AFR[0] |= GPIO_AFRL_AFRL0_0;
    printf("GPIO init");

}

static void _pulse_measure_timer_init(void){
    //Enable bus for TIM6
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    // with default clock speed of 16MHz, we want counter to increment every .1ms 
    TIM2->PSC = 1599;
    TIM2->ARR = 150;
    // Map IC1 (input capture1) to TI1 (timer interrupt1), sets CC1 as input for TIM2 
    TIM2->CCMR1 |= (TIM_CCMR1_CC1S_0);
    // Enable Capture/Compare 1 interrupt 
    TIM2->DIER |= TIM_DIER_CC1IE;
    // Configure events to trigger on rising and falling edges and enable
    // Capture Compare channel 1
    TIM2->CCER |= (TIM_CCER_CC1P | TIM_CCER_CC1NP | TIM_CCER_CC1E); 
    NVIC_EnableIRQ(TIM2_IRQn);
    TIM2->CR1 |= TIM_CR1_CEN;

    printf("timer init");
}

int pulse_measure_init(){
    _pulse_measure_gpio_init();
    _pulse_measure_timer_init();
    return 0;
}
