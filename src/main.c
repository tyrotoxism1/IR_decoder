#include "pulse_measure.h"
#include "decoder_NEC.h"
#include "uart.h"
#include "printf.h"
#ifndef _PRINTF_H_
#error "printf.h not found!"
#endif

int main(void)
{
  UART_config();
	decoder_NEC_init();
  // 1 = .1ms of resolution, 10 = 10ms for timeout of transmission burst 
	pulse_measure_init(1,10);
  GPIOA->MODER |= GPIO_MODER_MODE5_0;

   while(1){
		if(pulse_measure_get_status() == PM_COMPLETE){
			decoder_NEC_process_buffer(pulse_measure_get_buf(), 0, pulse_measure_get_edge_count());
			pulse_measure_print_values(1);
			decoder_NEC_print_data(); 
			GPIOA->ODR ^= GPIO_ODR_OD5;
			pulse_measure_set_status(PM_IDLE);
		}
	}
	return 0;
}

