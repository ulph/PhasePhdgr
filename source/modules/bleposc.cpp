#include "bleposc.hpp"
#include <string.h>
#include "sinc.hpp"
#include "inlines.hpp"

const int c_numFraction = 1000;
const auto c_blitTable = FractionalSincTable(BlitOsc::c_blitN, c_numFraction, (float)M_PI);
// TODO; investigate something like fs/4 bandlimit on the sinc itself...

BlitOsc::BlitOsc()
    : buf{0.f}
    , blit{0.f}
    , bufPos(0)
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
    // TODO invastigating aliasing and popping on sync
    // 1 check fractions are withing bounds ... and their calcs
    // 2 check sync fracion, and the resetting of phase
    // 3 ... leak compensate sync reset value? not by the delta, but target value itself...

    float freq = limit(inputs[0].value, 0.0f, float(fs)*0.5f);
    float shape = limit(inputs[1].value);
    float pwm = limit(inputs[2].value, 0.0f, 1.0f); // TODO weird shit happens on -1 when syncing
    float newSync = inputs[3].value;

    float nFreq = 2.f*freq/(float)fs; // TODO get this from inBus wall directly
    float bias = nFreq;

    if(nFreq == 0) return;

    // TODO, bias should also be integrated from some (perhaps order 1 is sufficient) derivative of freq
    // TODO, triangles (using derivative...)

    float leak = 1.f-nFreq*0.1; // maybe leak here can be somewhat adjustable?

    internalPhase += nFreq;

    if(newSync >= 0 && sync < 0){
        stage = -1;
    }

    while(true){
        if(stage==-1){
            // TODO - softsync
            float fraction = 0.0f;
            if(newSync != sync){
                fraction = (0 - sync) / (newSync - sync);
            }
            float t = -1.0f; // target value
            float r = cumSum; // current (future) value
            for(int n=0; n<c_blitN; ++n){
                r += buf[(bufPos+n)%c_blitN];
            }
            c_blitTable.getCoefficients(fraction, &blit[0], c_blitN);
            for(int n=0; n<c_blitN; ++n){
                buf[(bufPos+n)%c_blitN] += (t-r)*blit[n];
            }
            internalPhase = -1.f; // should take into account fraction here?
            stage = 0;
        }
        if(stage==0){
            if(internalPhase < pwm) break;
            float fraction = (pwm - (internalPhase-nFreq)) / nFreq;
            c_blitTable.getCoefficients(fraction, &blit[0], c_blitN);
            for(int n=0; n<c_blitN; ++n){
                buf[(bufPos+n)%c_blitN] += 2.f*shape*blit[n];
            }
            stage=1;
        }
        if(stage==1){
            if(internalPhase < 1.0f) break;
            float fraction = (1.f - (internalPhase-nFreq)) / nFreq;
            c_blitTable.getCoefficients(fraction, &blit[0], c_blitN);
            for(int n=0; n<c_blitN; ++n){
                buf[(bufPos+n)%c_blitN] -= 2.f*blit[n];
            }
            stage=0;
            internalPhase -= 2.0f;
        }
    }

    sync = newSync;

    cumSum = cumSum*leak + buf[bufPos] + (1-shape)*bias;
    // TODO, first order HP with cutoff quite low, like 5 hz or so. that or figure out why there is still some bias for higher notes
    outputs[0].value = cumSum;
    buf[bufPos] = 0.f;
    bufPos++;
    bufPos %= c_blitN;
}
