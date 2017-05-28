#include "bleposc.hpp"
#include <string.h>
#include "sinc.hpp"
#include "inlines.hpp"
#include "rlc.hpp"

const int c_numFraction = 1000;
const auto c_blitTable = FractionalSincTable(BlitOsc::c_blitN, c_numFraction, (float)M_PI, true);
// TODO; investigate something like fs/4 bandlimit on the sinc itself...

BlitOsc::BlitOsc()
    : buf{0.f}
    , blit{0.f}
    , bufPos(0)
    , cumSum(0.f)
    , stage(0)
    , internalPhase(0.f)
    , masterPhase(0.f)
    , last_nFreq(0.f)
    , last_pwm(0.f)
    , last_shape(0.f)
    , last_cumSum(0.f)
{
    inputs.push_back(Pad("freq"));
    inputs.push_back(Pad("shape")); // triangle <-> saw <-> square
    inputs.push_back(Pad("pwm")); // assymetry, rather. does not affect a pure saw
    inputs.push_back(Pad("reset")); // for osc sync
    inputs.push_back(Pad("sync")); // soft-sync
#if BLEP_DEBUG_PORTS
    inputs.push_back(Pad("debug_leak", 1.f));
    inputs.push_back(Pad("debug_hp", 1.f));
#endif
    outputs.push_back(Pad("output"));
}

void BlitOsc::process(uint32_t fs)
{
    const float c_slew = 0.9f; // a bit arbitrary...

    float freq = limit(inputs[0].value, 0.0f, float(fs)*0.5f);
    float shape = limit(inputs[1].value, 0.0f, 1.0f);
    float pwm = limit(inputs[2].value, 0.0f, 1.0f); // TODO weird shit happens on -1 when syncing
    float newMasterPhase = inputs[3].value;
    float syncAmount = 2.f*limit(inputs[4].value, 0.f, 1.f) - 1.f;

    float nFreq = 2.f*freq/(float)fs; // TODO get this from inBus wall directly

    // slew stuff -- workaround for jittery automation and simplistic aa
    nFreq = c_slew*last_nFreq + (1-c_slew)*nFreq;
    pwm = c_slew*last_pwm + (1-c_slew)*pwm;
    shape = c_slew*last_shape + (1-c_slew)*shape;
    last_pwm = pwm;
    last_shape = shape;
    last_nFreq = nFreq;

    if(nFreq == 0) return; // nothing to do, just exit

    float prop_leak = nFreq*0.1f;
#if BLEP_DEBUG_PORTS
    prop_leak *= inputs[5].value;
#endif
    float leak = 1.f-prop_leak;

    if( (internalPhase >= syncAmount) && newMasterPhase < 0 && masterPhase > 0){
        float unwrapperNewMasterPhase = newMasterPhase + 2.f;
        float syncFraction = (1.f - masterPhase) / (unwrapperNewMasterPhase - masterPhase);
        float t = -1.0f; // target value
        float r = cumSum; // current value
        // accumulate future value
        for(int n=0; n<c_blitN; ++n){
            r += buf[(bufPos+n)%c_blitN];
        }
        c_blitTable.getCoefficients(syncFraction, &blit[0], c_blitN);
        for(int n=0; n<c_blitN; ++n){
            buf[(bufPos+n)%c_blitN] += (t-r)*blit[n];
        }
        internalPhase = newMasterPhase;
        stage = 0;
    }

    internalPhase += nFreq;

    while(true){
        if(stage==0){
            if(internalPhase <= pwm) break;
            float interval = (pwm - (internalPhase-nFreq));
            // as pwm can be modulated, we could potetionally end up (slightly) outside the range on higer pitches
            while(interval > 1.f) interval -= nFreq;
            while(interval < 0.f) interval += nFreq;
            float fraction = interval / nFreq;
            c_blitTable.getCoefficients(fraction, &blit[0], c_blitN);
            for(int n=0; n<c_blitN; ++n){
                buf[(bufPos+n)%c_blitN] += 2.f*shape*blit[n];
            }
            stage=1;
        }
        if(stage==1){
            if(internalPhase <= 1.0f) break;
            float fraction = (1.f - (internalPhase-nFreq)) / nFreq;
            c_blitTable.getCoefficients(fraction, &blit[0], c_blitN);
            for(int n=0; n<c_blitN; ++n){
                buf[(bufPos+n)%c_blitN] -= 2.f*blit[n];
            }
            stage=0;
            internalPhase -= 2.0f;
        }
    }

    masterPhase = newMasterPhase;

    last_cumSum = cumSum; // x
    cumSum = cumSum*leak + buf[bufPos] + (1-shape)*nFreq; // x+1

    float fc = freq*0.125f;
#if BLEP_DEBUG_PORTS
    fc *= inputs[6].value;
#endif

    outputs[0].value = CalcRcHp(cumSum, last_cumSum, outputs[0].value, fc, fs); // todo, make this a parameter. closer to f0 may be beneficial when syncing, but lower _may_ be desireable when passing into a overdriven filter.
    buf[bufPos] = 0.f;
    bufPos++;
    bufPos %= c_blitN;
}
