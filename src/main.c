#include "pulse_measure.h"
#include "uart.h"
#include "printf.h"
#ifndef _PRINTF_H_
#error "printf.h not found!"
#endif

int main(void)
{
    UART_config();
    // 15 is timeout time in milliseconds for transmission 
    pulse_measure_init(15);
    while(1){
    }
    return 0;
}