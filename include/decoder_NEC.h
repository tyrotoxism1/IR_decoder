#ifndef DECODER_NEC_H
#define DECODER_NEC_H
#include <inttypes.h>

//opaque pointer
typedef struct decoder_NEC_t decoder_NEC_t;
typedef decoder_NEC_t* decoder_NEC_handler;

void decoder_NEC_process_buffer(uint32_t *buffer, uint8_t fast_parse, uint32_t pulse_cnt);
void decoder_NEC_print_data(void);
void decoder_NEC_init(void);

#endif //DECODER_NEC_H
