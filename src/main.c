#include "pulse_measure.h"
#include "decoder_NEC.h"
#include "uart.h"
#include "printf.h"
#ifndef _PRINTF_H_
#error "printf.h not found!"
#endif

// Used for testing decoder module  
static uint32_t example_timer_buf[150] = {89,45,6,16,6,5,6,5,6,5,6,5,6,5,6,5,6,5,6,5,6,16,6,16,5,17,5,16,6,16,6,16,6,16,5,\
16,6,5,6,16,6,16,6,5,6,5,6,5,7,5,6,5,6,16,6,5,6,5,6,16,6,16,6,17,6,16,6,16,6,5,6,16,6,16,5,6,6,5,6,6,6,\
5,6,5,6,17,6,5,6,5,6,16,6,16,6,16,6,17,6,0
};

static uint32_t example_repeat_command[150] = {
90,22,6,16,6,5,7,4,7,4,6,5,6,5, 
6,5,6,5,6,5,6,16,6,17,6,16,6,16,6,
16,7,16,6,16,6,16,6,5,7,16,6,16,6,5,
6,5,6,5,6,5,7,4,6,17,6,5,6,5,6,
16,6,16,5,16,6,16,6,16,6,5,7,16,6,16,
6,6,6,6,6,7,6,5,6,5,6,16,6,5,7,
4,5,16,6,16,6,16,6,16,6,131,90,22,6,0
};

int main(void)
{
    int printed = 0; 
	int pulse_measure_was_active = 0;
    UART_config();
	decoder_NEC_init();
    // 1 = .1ms of resolution, 15 = 15ms for timeout of transmission burst 
	pulse_measure_init(1,15);
    GPIOA->MODER |= GPIO_MODER_MODE5_0;

    while(1){

		if(pulse_measure_get_status() == PM_COMPLETE){
			decoder_NEC_process_buffer(pulse_measure_get_buf(), 0, pulse_measure_get_edge_count());
			pulse_measure_print_values(1);
			//decoder_NEC_print_data(); 
			GPIOA->ODR ^= GPIO_ODR_OD5;
			pulse_measure_set_status(PM_IDLE);
		}
	return 0;
}


/* Plus button example array data(rest would be filled with 0s)
89  45  6  16  6  5  6  5  6  5  6  5  6  5
 6  5  6  5  6  5  6  16  6  16  5  17  5  16  6 
 16  6  16  6  16  5  16  6  5  6  16  6  16  6  5
 6  5  6  5  7  5  6  5  6  16  6  5  6  5  6
 16  6  16  6  17  6  16  6  16  6  5  6  16  6  16
 5  6  6  7  6  6  6  5  6  5  6  17  6  5  6 
 5  6  16  6  16  6  16  6  17  6  0
*/

/* Plus button REPEAT example array data
90  22  6  16  6  5  7  4  7  4  6  5  6  5 
 6  5  6  5  6  5  6  16  6  17  6  16  6  16  6
 16  7  16  6  16  6  16  6  5  7  16  6  16  6  5
 6  5  6  5  6  5  7  4  6  17  6  5  6  5  6
 16  6  16  5  16  6  16  6  16  6  5  7  16  6  16 
 6  6  6  6  6  7  6  5  6  5  6  16  6  5  7
 4  5  16  6  16  6  16  6  16  6  131  90  22  6  0
*/

/* Plus button normal signal with .01 timer resultion
 293  892  450  59  163  57  54  60  51  60  51  59  51  60  50 
 59  51  59  50  59  50  59  162  59  162  60  162  60  162  60
 162  59  162  59  162  59  162  59  51  60  162  59  162  59  51 
 59  51  59  51  59  51  59  51  59  162  59  51  60  51  60
 162  59  162  59  162  59  162  59  162  59  51  59  162  59  162
 59  66  59  66  59  66  59  51  59  51  59  162  59  52  59 
 52  59  163  59  163  59  162  59  162  58  0  0  0  0  0
*/


/*
static uint32_t *example_timer_buf = {89,45,6, //setup
//3 1  x 0 x 0 x 0 x 0 x 0 x 0 x 0 x        address low
    16,6,5,6,5,6,5,6,5,6,5,6,5,6,5,6,
//19 0 x  1 x  1 x  1 x  1 x  1 x  1 x  1 x inverted address low
     5,6,16,6,16,5,17,5,16,6,16,6,16,6,16,5,

//35 1 x 0 x  1 x  1 x 0 x 0 x 0 x 0 x      address high
    16,6,5,6,16,6,16,6,5,6,5,6,5,7,5,6,
//  0 x  1 x 0 x 0 x  1 x  1 x  1 x  1 x    inverted address high 
    5,6,16,6,5,6,5,6,16,6,16,6,17,6,16,6,   

    //   1 x 0 x  1 x  1 x 0 x 0 x 0 x          command 
    16,6,5,6,16,6,16,5,6,6,7,6,6,6,
    //  0 x 0 x  1 x 0 x 0 x  1 x  1 x  1 x     inverse command
    5,6,5,6,17,6,5,6,5,6,16,6,16,6,16,6,
17,6,0
}
*/
