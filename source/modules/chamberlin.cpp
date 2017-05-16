#include "chamberlin.hpp"
#include <math.h>
#include "inlines.hpp"

ChamberlinFilter::ChamberlinFilter() 
    : band(0.f), low(0.f), high(0.f)
{
    inputs.push_back(Pad("input"));
    inputs.push_back(Pad("wc", 100.f));
    inputs.push_back(Pad("q"));
    outputs.push_back(Pad("low"));
    outputs.push_back(Pad("high"));
    outputs.push_back(Pad("band"));
    outputs.push_back(Pad("notch"));
}

void ChamberlinFilter::process(uint32_t fs) {
    float x = inputs[0].value;
    float wc = limit(inputs[1].value, 0.f, fs * 0.125f);
    float q = 1.f - limit(inputs[2].value, 0.f, 1.f);
    float f = 2.f * sinf((float)M_PI*wc / (float)fs);
    low += f * band;
    high = x - low - q*band;
    band += f*high;
    float notch = high + low;
    outputs[0].value = low;
    outputs[1].value = high;
    outputs[2].value = band;
    outputs[3].value = notch;
}
