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
    , sync(0.f)
{
    inputs.push_back(Pad("freq"));
    inputs.push_back(Pad("shape")); // 2saw <-> saw <-> square ... (a 'serendipity' with 2saw, cool stuff)
    inputs.push_back(Pad("pwm")); // pulse/saw <-> square/2saw <-> pulse/saw ...
    inputs.push_back(Pad("reset")); // for osc sync
    outputs.push_back(Pad("output"));
}

void BlitOsc::process(uint32_t fs)
{
    float freq = inputs[0].value;
    float shape = limit(inputs[1].value, 0.f, 1.f);
    float pwm = limit(inputs[2].value);
    float newSync = inputs[3].value;

    float nFreq = 2.f*freq/(float)fs;
    nFreq = (nFreq > 1.0f) ? 1.0f : nFreq;
    nFreq = (nFreq <-1.0f) ?-1.0f : nFreq;
    float bias = nFreq/2.0f;
    // TODO, bias should also be integrated from some (perhaps order 1 is sufficient) derivative of freq

    float leak = 1.f-nFreq*0.5; // TODO, maybe leak here can be somewhat adjustable?

    internalPhase += nFreq;

    if(newSync >= 0 && sync < 0){
        stage = -1;
    }

    while(true){
        if(stage==-1){
            // TODO - softsync
            // TODO - target value and reset phase not correct?
            float fraction = 0.0f;
            if(newSync != sync){
                fraction = (0 - sync) / (newSync - sync);
            }
            float t = -0.5f; // target value
            float r = cumSum; // current (future) value
            for(int n=0; n<c_blitN; ++n){
                r += buf[(bufPos+n)%c_blitN]; // should be bias here?
            }
            c_blitTable.getCoefficients(fraction, &blit[0], c_blitN);
            for(int n=0; n<c_blitN; ++n){
                buf[(bufPos+n)%c_blitN] += (t-r)*blit[n];
            }
            internalPhase = -1.f;
            stage = 0;
        }
        if(stage==0){
            if(internalPhase < pwm) break;
            float fraction = 0.0f;
            if(internalPhase != oldPhase){
                fraction = (pwm - oldPhase) / (internalPhase - oldPhase);
            }
            c_blitTable.getCoefficients(fraction, &blit[0], c_blitN);
            for(int n=0; n<c_blitN; ++n){
                buf[(bufPos+n)%c_blitN] += shape*blit[n];
            }
            stage=1;
        }
        if(stage==1){
            if(internalPhase < 1.0f) break;
            float fraction = 0.0f;
            if(internalPhase != oldPhase){
                fraction = (1.f - oldPhase) / (internalPhase - oldPhase);
            }
            c_blitTable.getCoefficients(fraction, &blit[0], c_blitN);
            for(int n=0; n<c_blitN; ++n){
                buf[(bufPos+n)%c_blitN] -= blit[n];
            }
            stage=0;
            internalPhase -= 2.0f;
        }
    }

    oldPhase = internalPhase;
    sync = newSync;

    cumSum = cumSum*leak + buf[bufPos] + (1-shape)*bias;
    outputs[0].value = cumSum;
    buf[bufPos] = 0.f;
    bufPos++;
    bufPos %= c_blitN;
}
