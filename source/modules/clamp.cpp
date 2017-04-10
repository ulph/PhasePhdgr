#include "clamp.hpp"

Clamp::Clamp()
{
    inputs.push_back(Pad("in"));
    outputs.push_back(Pad("clamp"));
}

void Clamp::process(uint32_t fs)
{
    outputs[0] = inputs[0];
    if(outputs[0].value < -1.0f) outputs[0].value = -1.0f;
    else if(outputs[0].value > 1.0f) outputs[0].value = 1.0f;
}
