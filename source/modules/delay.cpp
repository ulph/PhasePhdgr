#include <string.h>
#include "delay.hpp"
#include "sinc.hpp"

const int c_sincDelayN = 128;
const int c_sincDelayNumFraction = 1000;
const auto c_delayFracSincTable = FractionalSincTable(c_sincDelayN, c_sincDelayNumFraction, (float)M_PI);
const float max_delay_t = 5.f;

Delay::Delay()
    : readPosition(0)
    , buffer(nullptr)
    , bufferSize(0)
{
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("time", 0.5f));
    inputs.push_back(Pad("gain", 0.5f));
    outputs.push_back(Pad("out"));
}

Delay::~Delay(){
    delete[] buffer;
}

void Delay::process(uint32_t fs) {
    // design a FIR from windowed sinc with fractional delay as an approx of ideal allpass

    float t = inputs[1].value;
    float g = inputs[2].value;

    t = (t < 0.f) ? 0.f : ( t > max_delay_t ? max_delay_t : t); // max 10s

    // account for filter delay
    int tapeSamples = (int)(t*fs) - c_sincDelayN;
    tapeSamples = tapeSamples < 0 ? 0 : tapeSamples;

    if(tapeSamples > bufferSize){
        delete[] buffer;
        bufferSize = 2*tapeSamples;
        buffer = new float[bufferSize]();
    }

    tapeSamples = ((tapeSamples+c_sincDelayN) > bufferSize) ? (bufferSize-c_sincDelayN) : tapeSamples;

    const int writePosition = (readPosition + tapeSamples);

    const float frac = t*fs-(int)(t*fs);

    // ... (interpolated) impulse response from a precalcuated buffer
    float coeffs[c_sincDelayN] = {0};
    const int N = c_delayFracSincTable.getCoefficients(frac, &coeffs[0], c_sincDelayN);

    // apply it on to write buffer (running convolution)
    for(int n=0; n<N; n++){
        buffer[(writePosition+n)%bufferSize] += coeffs[n]*g*inputs[0].value;
    }

    outputs[0].value = buffer[readPosition];
    buffer[readPosition] = 0;
    readPosition++;
    readPosition %= bufferSize;
}
