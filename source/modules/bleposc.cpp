#include "bleposc.hpp"
#include <string.h>
#include "sinc.hpp"

const int c_numFraction = 1000;
const auto c_blitTable = FractionalSincTable(BlitOsc::c_blitN, c_numFraction, (float)M_PI);

BlitOsc::BlitOsc()
    : buf{0.f}
    , blit{0.f}
    , bufPos(0)
    , oldPhase(0.f)
    , cumSum(0.f)
{
    inputs.push_back(Pad("phase"));
    inputs.push_back(Pad("pwm"));
    inputs.push_back(Pad("shape")); // square vs saw
    outputs.push_back(Pad("output"));
}

void BlitOsc::process(uint32_t fs)
{
    const float leak = 0.99f;

    float phase = inputs[0].value;
    if(phase >= 0.f && oldPhase < 0.f){
        float fraction = - oldPhase / (phase - oldPhase);
        c_blitTable.getCoefficients(fraction, &blit[0], c_blitN);
        for(int n=0; n<c_blitN; ++n){
            buf[(bufPos+n)%c_blitN] += blit[n];
        }
    }
    else if(phase <= 0 && oldPhase > 0){
        float phase_ = phase + 1.0f;
        float oldPhase_ = oldPhase - 1.0f;
        float fraction = -oldPhase_ / (phase_ - oldPhase_);
        c_blitTable.getCoefficients(fraction, &blit[0], c_blitN);
        for(int n=0; n<c_blitN; ++n){
            buf[(bufPos+n)%c_blitN] -= blit[n];
        }
    }
    oldPhase = phase;

    cumSum = cumSum*leak + buf[bufPos];
    outputs[0].value = cumSum;
    buf[bufPos] = 0.f;
    bufPos++;
    bufPos %= c_blitN;
}
