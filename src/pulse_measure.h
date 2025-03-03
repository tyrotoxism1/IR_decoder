#ifndef PULSE_MEASURE_H
#define PULSE_MEASURE_H

//opaque pointer
typedef struct pulse_measure_t pulse_measure_t;
typedef pulse_measure_t* pulse_measure_handle;

//=======================================================
//By default use TIM2, configure in input mode using input
// channel 1, and will use PA0 with AF1 (based on Table 11 of STM32F446RE datasheet).
// Method configures Timer Input to TI1, rising and falling edge trigger,
// and enable corresponding capture compare channel   
int pulse_measure_init();
#endif