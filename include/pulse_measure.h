/**  @pulse_measure.h
* 
* @breif: Uses TIM2 to capture timings between edges of incoming IR signal from
* PA0. User of module shall provide the expected number of edges for a full transmission
* and repeat transmission. Using those parameters, the module when the tranmission 
* is complete. The module provides the timing data via an array.  
* 
* @note: Unsure on how to handle knowing when transmission is complete since the incomming
* transmission could be original or repeat type. The main differentiator should be the 2nd
* pulse. For original transmission, it's ~4.5ms while repeat is 2.25ms. 
*  (though this is just for NEC protocol). Ideally the pulse measure module doesn't have any
* logic about specific timings since that ruins encapsulation
*/
#ifndef PULSE_MEASURE_H
#define PULSE_MEASURE_H
#include <inttypes.h>
#include <math.h>

//opaque pointer
typedef struct pulse_measure_t pulse_measure_t;
typedef pulse_measure_t* pulse_measure_handle;

void pulse_measure_init(uint32_t timeout_ms);
uint32_t pulse_measure_get_edge_count(void);
uint32_t pulse_measure_get_tranmission_active(void);
void pulse_measure_reset(void);
void pulse_measure_active_check(void);
void pulse_measure_print_values(int print_array);
uint32_t pulse_measure_get_buf_val(uint16_t index);

#endif // PULSE_MEASURE_H