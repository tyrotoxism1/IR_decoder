#include "pulse_measure.h"
#include "uart.h"
#include "printf.h"
#ifndef _PRINTF_H_
#error "printf.h not found!"
#endif

int main(void)
{
    UART_config();
    // 1 = .1ms of resolution, 15 = 15ms for timeout of transmission burst 
    pulse_measure_init(1,15);

    while(1){
		if(pulse_measure_get_status() == PM_COMPLETE){
			pulse_measure_print_values(1);
			pulse_measure_set_status(PM_IDLE);
		}

    }
    return 0;
}
