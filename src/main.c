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
    //GPIOA->ODR &= ~(GPIO_ODR_OD4); 
    pulse_measure_print_values(1); 
    while(1){
        pulse_measure_active_check();
        if(!pulse_measure_get_tranmission_active()){
            if(pulse_measure_get_buf_val(1)){
                printf("%u",pulse_measure_get_buf_val(0));
                printf("\t%u",pulse_measure_get_buf_val(1));
                printf("\t%u\n",pulse_measure_get_buf_val(2));
            }
            pulse_measure_reset();
        }


    }
    return 0;
}