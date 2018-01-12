#define _USE_MATH_DEFINES
#include <math.h>

#include "sine.hpp"

Sine::Sine()
{
    inputs.push_back(Pad("phase"));
    outputs.push_back(Pad("sine"));
}

void Sine::process()
{
    outputs[0].value = sinf(float(M_PI) * inputs[0].value);
}

void Sine::block_process()
{
    for (size_t i = 0; i < Pad::k_blockSize; ++i) {
        outputs[0].values[i] = sinf(float(M_PI) * inputs[0].values[i]);
    }
}
