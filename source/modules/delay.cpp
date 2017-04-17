#include <string.h>
#include <assert.h>
#include "delay.hpp"
#include "sinc.hpp"

Delay::Delay() 
    : readPosition(0)
{
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("time", 0.5f));
    inputs.push_back(Pad("gain", 0.5f));
    outputs.push_back(Pad("out"));
    memset(buffer, 0, sizeof(buffer));
}

void Delay::process(uint32_t fs) {
    float t = inputs[1].value;
    float g = inputs[2].value;

    // limit time ranges
    t = t > 5.0f ? 5.0f : t < 0.0f ? 0.0f : t;

    int tapeSamples = (int)(t*fs) - g_fracSincTable.N;
    tapeSamples = tapeSamples < 0 ? 0 : tapeSamples;

    const int writePosition = (readPosition + tapeSamples);

    // design a FIR from windowed sinc with fractional delay as an approx of ideal allpass
    const float frac = t*fs-(int)(t*fs);
    assert(frac < 1.0f && frac >= 0.0f);

    // linearly interpolate between neighbouring sets of coefficients
    const float softIdx = frac * (float)g_fracSincTable.numFractions;
    assert(softIdx < g_fracSincTable.numFractions);
    const int tableIdx1 = (int)softIdx;
    const int tableIdx2 = ((tableIdx1+1) < g_fracSincTable.numFractions) ? (tableIdx1+1) : 0;
    assert(tableIdx1 < g_fracSincTable.numFractions);
    assert(tableIdx2 < g_fracSincTable.numFractions);

    const float ratio = softIdx - tableIdx1;
    assert(ratio < 1.0f && ratio >= 0.0f);

    // apply it on to write buffer (running convolution)
    int bufferSize = sizeof(buffer) / sizeof(float);
    for(int n=0; n<g_fracSincTable.N; n++){
        float c = (1 - ratio) * g_fracSincTable.coeffs[tableIdx1][n] + ratio * g_fracSincTable.coeffs[tableIdx2][n];
        buffer[(writePosition+n)%bufferSize] += c*g*inputs[0].value;
    }

    outputs[0].value = buffer[readPosition];
    buffer[readPosition] = 0;
    readPosition++;
    readPosition %= bufferSize;
}
