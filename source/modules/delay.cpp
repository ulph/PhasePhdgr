#include <string.h>

#include "delay.hpp"


Delay::Delay() 
    : readPosition(0)
{
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("time", 0.5f));
    inputs.push_back(Pad("gain", 0.5f));
    outputs.push_back(Pad("out"));
    memset(buffer, 0, sizeof(buffer));
}

float sincf(float x){
    return x!=0 ? sinf(x)/x : 1;
}

void Delay::process(uint32_t fs) {
    int bufferSize = sizeof(buffer) / sizeof(float);
    float t = inputs[1].value;
    float g = inputs[2].value;

    // limit time ranges
    t = t > 5.0f ? 5.0f : t < 0.0f ? 0.0f : t;

    const int N = 5;
    int tapeSamples = (int)(t*fs) - N;
    tapeSamples = tapeSamples < 0 ? 0 : tapeSamples;

    int writePosition = (readPosition + tapeSamples);
    writePosition %= bufferSize;

    // fractionally resample (needed for modulating time)
    float frac = t*fs-(int)(t*fs);

    // design a FIR from windowed sinc with fractional delay as an approx of ideal allpass
    float coeffs[N] = {0};

    float M = 0;
    for(int m=0; m<N; m++){
        float arg = (float)m-frac-((float)N-1.f)/2.f;
        float hamming = 0.54f+0.46f*cosf( (2.f*M_PI*arg) / (float)(N-1) );
        coeffs[m] = hamming*sincf(arg);
        M += coeffs[m];
    }

    // apply it
    for(int m=0; m<N; m++){
        buffer[(writePosition+m)%bufferSize] += g*inputs[0].value * coeffs[m] / M;
    }

    outputs[0].value = buffer[readPosition];
    buffer[readPosition] = 0;
    readPosition++;
    readPosition %= bufferSize;
}
