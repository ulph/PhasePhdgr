#include "sine.hpp"

Sine::Sine()
{
    inputs.push_back(Pad("phase"));
    outputs.push_back(Pad("sine"));
}

void Sine::process(uint32_t fs)
{
    outputs[0].value = (float)sin(M_PI * inputs[0].value);
}
