//#include "pulse_measure.h"
#include "uart.h"
#include "printf.h"
#ifndef _PRINTF_H_
#error "printf.h not found!"
#endif

int main(void){
    UART_config();
    while(1){
        printf("hello\n");
        //dumb delay
        for(volatile int i=0; i<100000; i++);
    }
    return 0;
}