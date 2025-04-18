/*
NOTES:

TODO: Double check what the edge index is of the buffer once the buffer is printed
    - We are getting more popluated values than expected and want to ensure we aren't getting unecessary data 
    or overwritting 
    - Not necessarily being overwritten unless transmission is repeat command, otherwise just forgot about low address 
    and high address being seperated

*/
#include "decoder_NEC.h"
#include "uart.h"
#include "printf.h"

// Probably change to static consts so we can use them in functions
static const uint32_t SETUP_START = 90;
static const uint32_t NEW_TRANSMISSION = 45;
static const uint32_t REPEAT_TRANSMISSION = 22;
static const uint32_t IR_DATA_1 = 16;
static const uint32_t IR_DATA_0 = 5;
static const uint32_t MARGIN_OF_ERROR = 1; 

typedef enum DECODE_STATUS {
    DECODE_ERROR,
    WAITING,
    IN_PROGRESS,
    COMPLETE,
} DECODE_STATUS;

/**
 * struct decoder_NEC_t - holds IR frame/transmission info populated from
 * parsing buffer of pulse timings.
 * @address: First 8 bits of IR frame.
 * @inverted_address: Second set of 8 bits from IR frame. Inverted bits of `address`, 
 *                    used for error detection.
 * @data: 3rd set of 8 bits from IR frame. `data` represents the command that should be
 *        executed. 
 * @inverted_data: 4th set of 8 bits. Inverted bits of `data` for error detection
 * @status: Indicates decoding status
 */
struct decoder_NEC_t{
    uint8_t address_low;
    uint8_t inverted_low_address;
    uint8_t address_high;
    uint8_t inverted_high_address;
    uint8_t command;
    uint8_t inverted_command;
    DECODE_STATUS status;
};

// Create global single instance of decoder module and init values to 0.(unsure
// if bad idea to set here and not in init funct
static decoder_NEC_t decoder_storage = {0};
decoder_NEC_handler decoder_inst; 

void decoder_NEC_init(void)
{
	decoder_inst = &decoder_storage;
	decoder_inst->status = WAITING;
}

/**
 * _within_margin() - Checks if value is within margin of error.
 * @value: data being checked 
 * @target_value: ideal value of passed data
 * @plus_minus: Margin of error `value` should be within
 * 
 * Returns boolean value based on `value` being within plus or minus the target
 * value inclusively.
 * 
 * Return: 
 * %0 - `value` is outside of margin of error
 * %1 - `value is within margin of error 
 */
uint8_t _within_margin(uint32_t value, uint32_t target_value, uint32_t plus_minus)
{
    uint32_t upper_range = target_value + plus_minus;
    uint32_t lower_range = target_value - plus_minus;
    if( (lower_range<=value) && (value<=upper_range) )
        return 1;
    else
        return 0;
}

/**
 * _populate_metadata() - Checks if timing buffer value is within timing range of 0 or 1.
 *                        If data is within margin of error, bit shift in corresponding data. 
 * 
 * @metadata: pointer to struct member of `decoder_NEC_t` struct.
 * @buffer: Array of timing values.
 * @start_idx: Starting point of `buffer` where data is destined for `metadata`. 
 * 
 * NEC Address and command are 8 bits, for each bit check the value in the buffer. If it's
 * within `IR_DATA_1` by a margin of 1 value (maybe margin should be variable) or within 
 * `IR_DATA_0` by a margin of 1, then bit shift corresponding value. Bit shift works by creating value
 * with single 1 in bit placement that correlates to bit in timing buffer. For example, `buffer[5]` is 
 * bit 1 of Address for NEC protocol (remember `buffer` holds timings of pulses in between data pulses) and lets
 * say `buffer[5]`=16. Value of 16 means data bit is 1, at Address bit 1 so the value `0b00000010` is OR'd with *metadata value. 
 * As the loop iterates over the value each buffer timing val is converted into either 0 or 1 and placed in respective bit. 
 */
static DECODE_STATUS _populate_metdata(uint8_t *metadata, uint32_t *buffer, int start_idx)
{	
/*
NOTE: bit shifting isn't working atm. We are shifting 

*/
    for(int i=0; i<16; i+=2){
		*metadata = (*metadata)<<1; 
        if( _within_margin(buffer[start_idx+i], IR_DATA_1, MARGIN_OF_ERROR) ){
            *metadata |= 1;
        }
        else if(!( _within_margin(buffer[start_idx+i], IR_DATA_0, MARGIN_OF_ERROR)) ){
            decoder_inst->status = DECODE_ERROR;
            return decoder_inst->status;
        }
        printf("i:%i\ttiming: %i\tvalue:%i\n", i,buffer[start_idx+i], *metadata);
    }
	return decoder_inst->status;
}

/**
 * decoder_NEC_process_buffer() - Iterate over array to poplulate module attributes
 * with incoming bits based on pulse measurment timings. 
 * 
 * @buffer: Pointer to the buffer of timing values. 
 * @fast_parse: Boolean that determines if inverted address and inverted command are ignored or
 *              not. If `fast_parse` is greater than 0 inverted bytes are ignored, 
 *              otherwise error detection ensures inverted and original bits are equal.
 * @pulse_cnt: Number of pulse timing values in `buffer`.
 * 
 * Context:  Incoming data shall be .1ms resolution, meaning a value of 90 = 9ms.
 * 
 * Iterates over timing values in `buffer` using passed number of pulse values. Determines if setup pulses 
 * indicate new or repeat transmission. If pulses indicate repeat, nothing extra is needed and returns. 
 * Otherwise, process buffer as so: iterate over every other pulse timing value since transmission 
 * includes 560us pulse between bits. If `fast_parse` is set, iteration skips over inverse byte (sent after address and command bits).
 */
void decoder_NEC_process_buffer(uint32_t *buffer, uint8_t fast_parse, uint32_t pulse_cnt)
{
    if( _within_margin(buffer[0], SETUP_START, MARGIN_OF_ERROR) ){
        if(_within_margin(buffer[1], REPEAT_TRANSMISSION, 1)){
            return;
        }
        else if(_within_margin(buffer[1], NEW_TRANSMISSION, MARGIN_OF_ERROR)){
            decoder_inst->status = IN_PROGRESS;             
        }
        else{
            decoder_inst->status = DECODE_ERROR;
        }
    }
    printf("low addr\n");
    if( (_populate_metdata( &(decoder_inst->address_low), buffer, 3)) == DECODE_ERROR)
	{
		printf("ERROR ocured processing low address\n");
		return;
	}
    printf("\ninverse low addr\n");
    if( (_populate_metdata( &(decoder_inst->inverted_low_address), buffer, 19)) == DECODE_ERROR)
	{
		printf("ERROR ocured processing inverse low address\n");
		return;
	}
    printf("\nhigh addr\n");
    if( (_populate_metdata( &(decoder_inst->address_high), buffer, 35)) == DECODE_ERROR)
	{
		printf("ERROR ocured processing high address\n");
		return;
	}
    printf("\ninverse high addr\n");
    if( (_populate_metdata( &(decoder_inst->inverted_high_address), buffer, 51)) == DECODE_ERROR)
	{
		printf("ERROR ocured processing inverse high address\n");
		return;
	}
    printf("\ncommand 1\n");
    if( (_populate_metdata( &(decoder_inst->command), buffer, 67)) == DECODE_ERROR)
	{
		printf("ERROR ocured processing command 1\n");
		return;
	}
    printf("\ncommand 2\n");
    if( (_populate_metdata( &(decoder_inst->inverted_command), buffer, 83)) == DECODE_ERROR)
	{
		printf("ERROR ocured processing command 2\n");
		return;
	}
	decoder_inst->status = COMPLETE;
}

DECODE_STATUS decoder_NEC_get_status(void)
{
    return decoder_inst->status;
}

void decoder_NEC_print_data(void)
{
    printf("STATUS: %i\n", decoder_inst->status);
    printf("Address low: %i\n", decoder_inst->address_low);
    printf("Address high: %i\n", decoder_inst->address_high);
    printf("Address command 1: %i\n", decoder_inst->command);
}



