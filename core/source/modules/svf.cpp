#include "svf.hpp"
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

void ChamberlinFilter::process() {
    const float stabF = 1.f / 6.f;
    float x = inputs[0].value;
    float wc = limit(inputs[1].value, 0.f, fs * stabF);
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

OpenChamberlinFilter::OpenChamberlinFilter()
{
    inputs.push_back(Pad("input"));
    inputs.push_back(Pad("wc", 100.f));
    inputs.push_back(Pad("q"));
    inputs.push_back(Pad("low"));
    inputs.push_back(Pad("high"));
    inputs.push_back(Pad("band"));
    outputs.push_back(Pad("low"));
    outputs.push_back(Pad("high"));
    outputs.push_back(Pad("band"));
    outputs.push_back(Pad("notch"));
}

void OpenChamberlinFilter::process() {
    const float stabF = 1.f / 6.f;
    float x = inputs[0].value;
    float wc = limit(inputs[1].value, 0.f, fs * stabF);
    float q = 1.f - limit(inputs[2].value, 0.f, 1.f);
    float f = 2.f * sinf((float)M_PI*wc / (float)fs);
    float low = inputs[3].value;
    float high = inputs[4].value;
    float band = inputs[5].value;
    low += f * band;
    high = x - low - q*band;
    band += f*high;
    float notch = high + low;
    outputs[0].value = low;
    outputs[1].value = high;
    outputs[2].value = band;
    outputs[3].value = notch;
}

// ...

TrapezoidalTanSVF::TrapezoidalTanSVF()
{
    inputs.push_back(Pad("input"));
    inputs.push_back(Pad("fc", 100.f));
    inputs.push_back(Pad("res"));
    outputs.push_back(Pad("low"));
    outputs.push_back(Pad("high"));
    outputs.push_back(Pad("band"));
    outputs.push_back(Pad("notch"));
    outputs.push_back(Pad("all"));
}

void TrapezoidalTanSVF::process() {
    // inputs
    float v0 = inputs[0].value;
    float fc = limit(inputs[1].value, 0.0f, 0.5f*fs);
    float res = limit(inputs[2].value, 0.0f, 1.0f);

    // design
    float g = tanf((float)M_PI*fc / fs);
    float k = 2.0f - 2.0f * res;
    float a1 = 1.f / (1.f + g*(g + k));
    float a2 = g*a1;
    float a3 = g*a2;

    // compute
    float v3 = v0 - ic2eq;
    float v1 = a1*ic1eq + a2*v3;
    float v2 = ic2eq + a2*ic1eq + a3*v3;

    // store states
    ic1eq = 2.0f * v1 - ic1eq;
    ic2eq = 2.0f * v2 - ic2eq;

    // store outputs
    float low = v2;
    float band = v1;
    float high = v0 - k*v1 - v2;
    float notch = low + high;
    float peak = low - high;
    float all = low + high - k*band;

    outputs[0].value = low;
    outputs[1].value = high;
    outputs[2].value = band;
    outputs[3].value = notch;
    outputs[4].value = all;
}
