#include <string.h>
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

    int tapeSamples = (int)(t*fs) - c_sincDelayN;
    tapeSamples = tapeSamples < 0 ? 0 : tapeSamples;

    const int writePosition = (readPosition + tapeSamples);

    // design a FIR from windowed sinc with fractional delay as an approx of ideal allpass
    const float frac = t*fs-(int)(t*fs);

    // get it from a precalcuated buffer
    float coeffs[c_sincDelayN] = {0};
    const int N = g_delayFracSincTable.getCoefficients(frac, &coeffs[0], c_sincDelayN);

    // apply it on to write buffer (running convolution)
    int bufferSize = sizeof(buffer) / sizeof(float);
    for(int n=0; n<N; n++){
        buffer[(writePosition+n)%bufferSize] += coeffs[n]*g*inputs[0].value;
    }

    outputs[0].value = buffer[readPosition];
    buffer[readPosition] = 0;
    readPosition++;
    readPosition %= bufferSize;
}
