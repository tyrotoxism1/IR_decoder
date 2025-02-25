#include "pulse_measure.hpp"
#include "stm32f4xx.h"
#include "uart.h"
#include "printf.h"

Pulse_measure::Pulse_measure(){
    //
    int init_result = input_capture_timer_init();
    //If timer init was successful, class is valid
    if(init_result>0){
        printf("success");

    }

};

//=======================================================
//By default use TIM2, configure in input mode using input
// channel 1, and will use PA0 with AF1 (based on Table 11 of STM32F446RE datasheet).
// Method configures Timer Input to TI1, rising and falling edge trigger,
// and enable corresponding capture compare channel   
int Pulse_measure::input_capture_timer_init(){
    // Map IC1 to TI1, sets CC1 as input for TIM2 
    TIM2->CCMR1 |= (TIM_CCMR1_CC1S_0);
    // Configure events to trigger on rising and falling edges and enable
    // Capture Compare channel 1
    TIM2->CCER |= (TIM_CCER_CC1P | TIM_CCER_CC1NP | TIM_CCER_CC1E); 
}

