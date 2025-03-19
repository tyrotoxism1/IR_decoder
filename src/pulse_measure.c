#include "pulse_measure.h"
#include "stm32f4xx.h"
#include "uart.h"
#include "printf.h"


#define PULSE_MEASURE_MAX_PULSES 150

/**
 * struct pulse_measure_t - holds incoming pulse measurnments and minimum necessary
 * supplimental info for transmission pulse measurnments 
 * @active_transmission: Indicates if transmission of currently occuring. Set by 
 * input capture interrupt and cleared when timeout interrupt occurs.
 * @timing_buf: Buffer of pulse measurnments. Timer resolution is 1ms so counter
 * values stored in buffer are a tenth of a milisecond.  
 * @edge_index: Count of number of edges from input capture interrupt. Acts as 
 * `timing_buf` index
 */
struct pulse_measure_t{
    uint32_t active_transmission;
    uint32_t timing_buf[PULSE_MEASURE_MAX_PULSES]; 
    uint32_t edge_index;
};
// Create global single instance of pulse measure module and init values to 0.
static pulse_measure_t pm_instance_storage = {0};
pulse_measure_handle pm_instance = &pm_instance_storage;

/**
 * TIM2_IRQHandler() - Interrupt handler for TIM2 interrupts. Handles input capture
 * and counter overflow interrupts
 * 
 * For input capture interrupt, save timer value and reset timer count. Overflow interrupt
 * means timeout occurred due to end of transmission or error during transmission. 
 */
void TIM2_IRQHandler(void)
{
    if(TIM2->SR & TIM_SR_CC1IF){
        GPIOA->ODR |= GPIO_ODR_OD4; 
        TIM2->SR &= ~TIM_SR_UIF;  
        TIM2->DIER |= TIM_DIER_UIE; 
        pm_instance->active_transmission = 1; 
        // Reading CCR1 clears TIM_SR_CC1F 
        pm_instance->timing_buf[(pm_instance->edge_index)++] = TIM2->CCR1;
        TIM2->CNT = 0;
    }
    if(TIM2->SR & TIM_SR_UIF){
        GPIOA->ODR &= ~(GPIO_ODR_OD4); 
        TIM2->DIER &= ~(TIM_DIER_UIE); 
        pm_instance->active_transmission = 0;
        pm_instance->edge_index = 0;
        //clear status register to acknowledge interrupt has fired
        TIM2->SR &= ~(TIM_SR_UIF);
    }
}

/**
 * _pulse_measure_gpio_init() - Intalizes PA0 to alternate function for TIM2 use. 
 * 
 * Default port used is PA0. TIM2 uses port in alternate function 1 mode for input
 * capture. 
 * 
 * TODO: Configure used port for input capture based on what's avaliable. TIM2 input capture
 * can use PA0, PA8, PA15, or PB8 in AF1
 */
static void _pulse_measure_gpio_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; 
    GPIOA->MODER |= GPIO_MODER_MODE0_1;
    // Use PA4 as debug LED, set it to be output mode 
    GPIOA->MODER |= GPIO_MODER_MODE4_0;
    GPIOA->AFR[0] &= ~(GPIO_AFRL_AFRL0);
    GPIOA->AFR[0] |= GPIO_AFRL_AFRL0_0;

}

/**
 * _pulse_measure_timer_init() - Set up TIM2 for input capture mode
 * @resolution: determines timer resolution, must be value from 0-3, with 0 
 *              representing lowest resolution and 3 being the highest resolution. 
 *              0 = 1ms resolution, MAX timeout ~= 60000ms
 *              1 = .1ms resolution, MAX timeout ~= 6000ms
 *              2 = .01ms resolution, MAX timeout ~= 600ms
 *              3 = .001ms resolution, MAX timeout ~= 60ms
 * @timeout: Set the amount of time (in ms) for the pulse measure module to reset back to inactive tranmsission.
 *           If no external pulse occurs within timeout period from last edge of input signal, timer assumes 
 *           transmission is complete (or error occurred) and resets.  
 *           Value is adjusted based on @resolution to equivalent value, then adjusted value is assigned to ARR. 
 *
 * Set TIM2 to input capture mode. The passed resolution determines when the counter increments meaning all values 
 * stored in `pm_instance->timer_buf` are of that resolution. For example, if `resolution` is 1 then counter increments 
 * every .1ms, thus if pulse is 9.2ms wide, stored value would be '92'. 
 * TODO: Have value of 15999 be based off of actual clock frequency
 * 
 * Return: Indicate status of timer intialization
 * * %0     - Initalized successfully
 * * %-1    - Parameter Error 
 */
static int _pulse_measure_timer_init(uint32_t resolution, uint32_t timeout){
    if(resolution<0 || resolution>3)
        return -1;
    
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->PSC = 15999/( (int)(pow(10,resolution)) );
    TIM2->ARR = timeout*(int)(pow(10,resolution));
    printf("PSC: %i\tARR: %i\n",TIM2->PSC, TIM2->ARR);
    // Map IC1 (input capture1) to TI1 (timer interrupt1), sets CC1 as input for TIM2 
    TIM2->CCMR1 |= (TIM_CCMR1_CC1S_0);
    TIM2->DIER &= ~(TIM_DIER_UIE); 
    TIM2->DIER |= TIM_DIER_CC1IE;
    // Configure events to trigger on rising and falling edges and enable Capture Compare channel 1
    TIM2->CCER |= (TIM_CCER_CC1P | TIM_CCER_CC1NP | TIM_CCER_CC1E); 
    NVIC_EnableIRQ(TIM2_IRQn);
    TIM2->CR1 |= TIM_CR1_CEN;

    return 0;
}


/**
 * pulse_measure_init() - Setup and configure GPIO and timer 
 * @resolution: determines timer resolution, must be value from 0-3, with 0 
 *              representing lowest resolution and 3 being the highest resolution. 
 *              0 = 1ms resolution, MAX timeout ~= 60000ms
 *              1 = .1ms resolution, MAX timeout ~= 6000ms
 *              2 = .01ms resolution, MAX timeout ~= 600ms
 *              3 = .001ms resolution, MAX timeout ~= 60ms
 * @timeout: Set the amount of time (in ms) for the pulse measure module to reset back to inactive tranmsission.
 *           If no external pulse occurs within timeout period from last edge of input signal, timer assumes 
 *           transmission is complete (or error occurred) and resets.  
 *           Value is adjusted based on @resolution to equivalent value, then adjusted value is assigned to ARR. 
 *
 * Set TIM2 to input capture mode. The passed resolution determines when the counter increments meaning all values 
 * stored in `pm_instance->timer_buf` are of that resolution. For example, if `resolution` is 1 then counter increments 
 * every .1ms, thus if pulse is 9.2ms wide, stored value would be '92'. 
 * GPIO for the input capture is set to Alternate Function 1 (AF1) allowing for port to directly connect to TIM2
 * TODO: Have value of 15999 be based off of actual clock frequency 
 * 
 * Return: Indicate status of timer intialization
 * * %0     - Initalized successfully
 * * %-1    - Parameter Error 
 */
int pulse_measure_init(uint32_t resolution, uint32_t timeout)
{
    _pulse_measure_gpio_init();
    return _pulse_measure_timer_init(resolution, timeout);
}

/**
 * pulse_measure_get_edge_count() - Accessor for number of edges occurred since last reset.
 * Ideally number correlates to number of edges occurred for current transmission.
 */
uint32_t pulse_measure_get_edge_count(void)
{
    return pm_instance->edge_index;
}

/**
 * pulse_measure_get_tranmission_active() - Accessor to state of transmission for pulse measure module. 
 */
uint32_t pulse_measure_get_tranmission_active(void)
{
    return pm_instance->active_transmission;
}

/**
 * pulse_measure_reset() - blocks interrupts while resetting pulse measurnment member values.
 * 
 * If error occurs or timer for pulse measurment timesout, this function should be called to 
 * reset/clear values. It is not called automatically. Function disables interrupts for TIM2
 * temporaily  
*/
void pulse_measure_reset(void)
{
    NVIC_DisableIRQ(TIM2_IRQn);
    TIM2->CR1 &= ~(TIM_CR1_CEN);

    for(int i=0; i<pm_instance->edge_index; i++){
        pm_instance->timing_buf[i] = 0;
    }

    pm_instance->active_transmission = 0;
    pm_instance->edge_index = 0;

    NVIC_EnableIRQ(TIM2_IRQn);
    TIM2->CR1 |= (TIM_CR1_CEN);
}

/**
 * pulse_measure_print_values() - Displays module's metadata and buffer values
 * 
 * @print_array: Prints all array values if nonzero, ignores printing otherwise 
 */
void pulse_measure_print_values(int print_array)
{
    printf("Active transmisssion: %i\n", pm_instance->active_transmission);
    printf("edge inded: %i\n", pm_instance->active_transmission);
    if(print_array){
        printf("Array values:");
        for(int i=0; i<PULSE_MEASURE_MAX_PULSES; i++){
            if(i%15==0){
                printf("\n");
            }
            printf(" %i ", pm_instance->timing_buf[i]);
        }
    }
}

/**
 * pulse_measure_get_buf_val() - Accessor of specific timing buffer index
 * 
 * @index: 0 based index of buffer
 */
uint32_t pulse_measure_get_buf_val(uint16_t index)
{
    return pm_instance->timing_buf[index];
}

/**
 * pulse_measure_get_buf() - Accessor to buffer pointer
 */
uint32_t* pulse_measure_get_buf(void)
{
    return pm_instance->timing_buf;
}