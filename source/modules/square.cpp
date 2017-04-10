#include "square.hpp"

Square::Square()
{
    inputs.push_back(Pad("phase"));
    inputs.push_back(Pad("dutycycle", 1.0f));
    outputs.push_back(Pad("square"));
}

void Square::process(uint32_t fs)
{
    // Get phase
    float phase = inputs[0].value;

    // Duty cycle (PWM)
    float duty = inputs[1].value;
    if(duty < 0.0f) duty = -duty;

    // Make Square from phase
    if(phase < -1.0f+duty) outputs[0].value = -1.0f;
    else if(phase < 0.0f) outputs[0].value =  0.0f;
    else if(phase < duty) outputs[0].value =  1.0f;
    else outputs[0].value =  0.0f;
}
