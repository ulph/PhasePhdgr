#include "saturatoratan.hpp"

SaturatorAtan::SaturatorAtan()
{
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("prescaler", 1.0f));
    outputs.push_back(Pad("saturated"));
}

void SaturatorAtan::process(uint32_t fs)
{
    float scale = fmaxf(inputs[1].value, 0.01);
    outputs[0].value = atanf(inputs[0].value * scale) / atanf(scale);
}
