#ifndef PULSE_MEASURE_H
#define PULSE_MEASURE_H

// ====================================================
// Pulse_measure class uses input capture mode of timers to measure 
//  timings between rise and fall IR signal. The timings are stored
//  in an array and handed off to the `IR_decoder` class. 
class Pulse_measure {
    private:
        static const int MAX_PULSES = 110;
        int pulse_timings[MAX_PULSES] = {0};
        bool initialized = 0;
        //Use TIM2 in input capture l 
        int input_capture_timer_init();
    public:
        Pulse_measure();    
};

#endif