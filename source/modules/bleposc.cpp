#include "bleposc.hpp"
#include <string.h>
#include "sinc.hpp"
#include "inlines.hpp"

const int c_numFraction = 1000;
const auto c_blitTable = FractionalSincTable(BlitOsc::c_blitN, c_numFraction, (float)M_PI);

BlitOsc::BlitOsc()
    : buf{0.f}
    , blit{0.f}
    , bufPos(0)
    , oldPhase(0.f)
    , cumSum(0.f)
    , stage(0)
    , internalPhase(0.f)
{
    inputs.push_back(Pad("freq"));
    inputs.push_back(Pad("shape")); // 2saw <-> saw <-> square ... (a 'serendipity' with 2saw, cool stuff)
    inputs.push_back(Pad("pwm")); // pulse/saw <-> square/2saw <-> pulse/saw ...
    outputs.push_back(Pad("output"));
}

void BlitOsc::process(uint32_t fs)
{
    float freq = inputs[0].value;
    float shape = limit(inputs[1].value);
    float pwm = limit(inputs[2].value);

    const float leak = 0.999f;
    float nFreq = 2.f*freq/(float)fs; // TODO, feed in nFreq from inBus as it'll be quite nice [-1,1]
    float bias = nFreq/2.0f;

    internalPhase += nFreq;

    if(stage==0 && internalPhase >= pwm){
        float fraction = 0.0f;
        if(internalPhase != oldPhase){
            fraction = (pwm - oldPhase) / (internalPhase - oldPhase);
        }
        int n = c_blitTable.getCoefficients(fraction, &blit[0], c_blitN);
        assert(n == c_blitN);
        for(int n=0; n<c_blitN; ++n){
            buf[(bufPos+n)%c_blitN] += shape*blit[n];
        }
        stage=1;
    }
    if(stage==1 && internalPhase >= 1.f){
        float fraction = 0.0f;
        if(internalPhase != oldPhase){
            fraction = (1.f - oldPhase) / (internalPhase - oldPhase);
        }
        int n = c_blitTable.getCoefficients(fraction, &blit[0], c_blitN);
        assert(n == c_blitN);
        for(int n=0; n<c_blitN; ++n){
            buf[(bufPos+n)%c_blitN] -= blit[n];
        }
        stage=0;
        internalPhase -= 2.0f;
    }

    oldPhase = internalPhase;

    cumSum = cumSum*leak + buf[bufPos] + (1-shape)*bias;
    outputs[0].value = cumSum;
    buf[bufPos] = 0.f;
    bufPos++;
    bufPos %= c_blitN;
}
