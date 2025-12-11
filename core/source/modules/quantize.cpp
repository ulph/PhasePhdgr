#include "quantize.hpp"
#include <math.h>

Quantize::Quantize()
{
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("factor", 1.f));
    outputs.push_back(Pad("out"));
}

void Quantize::processSample(int sample)
{
    float bits = inputs[1].values[sample]*15.f; // one bit for sign

    const float invStepSize = powf(2.f, bits);
    const float stepSize = 1.0f / invStepSize;

    float tmp = roundf(inputs[0].values[sample] * invStepSize); // and one less for positive max

    tmp = tmp * stepSize;

    outputs[0].values[sample] = tmp;
}
