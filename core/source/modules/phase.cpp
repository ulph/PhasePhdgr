#include "phase.hpp"

Phase::Phase() : trig(0.f){
    inputs.push_back(Pad("freq"));
    inputs.push_back(Pad("reset"));
    outputs.push_back(Pad("phase"));
}

void Phase::process(uint32_t fs)
{
    // Get phase
    float f = inputs[0].value;
    float p = outputs[0].value;
    float t = inputs[1].value;
    if(t>=0.f && trig<0.f)
        p = 0;
    else if(fs){
        p += 2 * f / (float)fs;
    }
    trig = t;
    while(p > 1){p-=2;}
    while(p < -1){p+=2;}
    outputs[0].value = p;
}
