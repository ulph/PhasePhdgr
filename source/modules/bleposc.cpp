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
    inputs.push_back(Pad("shape")); // square <-> saw ...
    inputs.push_back(Pad("pwm")); // square <-> pulse ...
    outputs.push_back(Pad("output"));
}

void BlitOsc::process(uint32_t fs)
{
    float phase = inputs[0].value;
    float shape = inputs[1].value;
    float pwm = inputs[2].value;

    const float leak = 0.999f;
    float phaseDiff = phase - oldPhase;
    phaseDiff += phaseDiff<0.f?2.0:0.f;
    float bias = phaseDiff/2.0f;

    if(phase >= pwm && oldPhase < pwm){
        float fraction = 0.0f;
        if(phase != oldPhase){
            fraction = (pwm - oldPhase) / (phase - oldPhase);
        }
        c_blitTable.getCoefficients(fraction, &blit[0], c_blitN);
        for(int n=0; n<c_blitN; ++n){
            buf[(bufPos+n)%c_blitN] += shape*blit[n];
        }
    }
    else if(phase <= 0 && oldPhase > 0){
        float phase_ = phase + 1.0f;
        float oldPhase_ = oldPhase - 1.0f;
        float fraction = 0.0f;
        if(phase_ != oldPhase_){
            fraction = -oldPhase_ / (phase_ - oldPhase_);
        }
        c_blitTable.getCoefficients(fraction, &blit[0], c_blitN);
        for(int n=0; n<c_blitN; ++n){
            buf[(bufPos+n)%c_blitN] -= blit[n];
        }
    }
    oldPhase = phase;

    cumSum = cumSum*leak + buf[bufPos] + (1-shape)*bias;
    outputs[0].value = cumSum;
    buf[bufPos] = 0.f;
    bufPos++;
    bufPos %= c_blitN;
}
