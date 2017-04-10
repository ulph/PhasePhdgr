#include "phase.hpp"

Phase::Phase() {
    inputs.push_back(Pad("freq"));
    outputs.push_back(Pad("phase"));
}

void Phase::process(uint32_t fs)
{
    // Get phase
    float f = inputs[0].value;
    float p = outputs[0].value;
    if(fs){
        p += 2 * f / (float)fs;
    }
    while(p > 1){p-=2;}
    while(p < -1){p+=2;}
    outputs[0].value = p;
}
