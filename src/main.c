#include "pulse_measure.h"
#include "uart.h"
#include "printf.h"
#ifndef _PRINTF_H_
#error "printf.h not found!"
#endif

int main(void){
    UART_config();
    pulse_measure_init();
    while(1){
    }
    return 0;
}