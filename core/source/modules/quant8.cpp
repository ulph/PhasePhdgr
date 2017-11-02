#include "quant8.hpp"

Quant8::Quant8()
{
    inputs.push_back(Pad("in"));
    outputs.push_back(Pad("quant"));
}

void Quant8::process(uint32_t fs)
{
    const float invStepSize = 128.0f;
    const float stepSize = 1.0f / invStepSize;

    float tmp = roundf(inputs[0].value * invStepSize);
    if(tmp < -invStepSize) tmp = -invStepSize;
    else if(tmp > invStepSize-1) tmp = invStepSize-1;
    tmp = tmp * stepSize;

    outputs[0].value = tmp;
}
