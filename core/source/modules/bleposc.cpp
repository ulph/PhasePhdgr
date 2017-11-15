#include "bleposc.hpp"
#include <string.h>
#include "sinc.hpp"
#include "inlines.hpp"
#include "rlc.hpp"

#define BLEP_DEBUG_PORTS 1

const auto c_blitTable = FractionalSincTable<8>();
// TODO; investigate something like fs/4 bandlimit on the sinc itself...

BlitOsc::BlitOsc()
    : buf{0.f}
    , blit{0.f}
    , bufPos(0)
    , cumSum(0.f)
    , stage(0)
    , internalPhase(0.f)
    , syncPhase(0.f)
    , last_cumSum(0.f)
{
    inputs.push_back(Pad("freq"));
    inputs.push_back(Pad("shape")); // saw <-> square
    inputs.push_back(Pad("pwm"));
    inputs.push_back(Pad("syncFreq")); // 'master' osc freq, for osc sync purposes
    inputs.push_back(Pad("sync")); // how much to sync -- TODO non-linear map input range
    inputs.push_back(Pad("reset")); // reset both internal phases ... not suitable for osc sync as it'll alias
    outputs.push_back(Pad("output"));
}

void BlitOsc::process(uint32_t fs)
{
    float freq = limit(inputs[0].value, 1.f, float(fs)*0.5f);
    float shape = limit(inputs[1].value, 0.0f, 1.0f);
    float pwm = limit(inputs[2].value);
    float syncFreq = inputs[3].value;
    float syncAmount = 2.f*(1.f-limit(inputs[4].value, 0.f, 1.f)) - 1.f;

    float nFreq = 2.f*freq/(float)fs; // TODO get this from inBus wall directly
    float syncNFreq = 2.f*syncFreq/(float)fs; // TODO get this from inBus wall directly

    if(nFreq == 0) return; // nothing to do, just exit

    float prop_leak = nFreq * 0.01f;
    float leak = 1.f - prop_leak;

    // reset the clocks on an upflank through zero
    float reset = inputs[5].value;
    if(reset>=0.f && last_reset<0.f){
        syncPhase = -1.f;
        internalPhase = -1.f;
    }
    last_reset = reset;

    syncPhase += syncNFreq;
    internalPhase += nFreq;

    while(true){
        if( stage == 0 ){
            if(internalPhase <= pwm) break;
            float interval = (pwm - (internalPhase-nFreq));
            // deal with modulated pwm (not exactly correct but good enough)
            while(interval > 1.f) interval -= nFreq;
            while(interval < 0.f) interval += nFreq;
            float fraction = interval / nFreq;
            c_blitTable.getCoefficients(fraction, &blit[0], c_blitN);
            for(int n=0; n<c_blitN; ++n){
                buf[(bufPos+n)%c_blitN] += 2.f*shape*blit[n];
            }
            stage = 1;
        }
        if( stage == 1 ){
            if(internalPhase <= 1.0f) break;
            float interval = (1.f - (internalPhase-nFreq));
            float fraction = interval / nFreq;
            c_blitTable.getCoefficients(fraction, &blit[0], c_blitN);
            for(int n=0; n<c_blitN; ++n){
                buf[(bufPos+n)%c_blitN] -= 2.f*blit[n];
            }
            stage = 0;
            internalPhase -= 2.0f;
        }
    }

    if(syncPhase > 1.f){
        bool resetPhase = false;
        if(internalPhase >= syncAmount){
            resetPhase = true;
            float interval = (1.f - (syncPhase - syncNFreq));
            float syncFraction =  interval / syncNFreq;

            float freqFractionCorrection = (syncNFreq/nFreq); // this helps a tiny bit extra, need to do the math as of why :P
            float sawCorrection = (1.f-shape)*nFreq*(1.f-syncFraction*freqFractionCorrection); // adjust for saw shape

            float target = -1.0f + sawCorrection; // target value

            float remainderTail = 0.f;
            for(int n=0; n<c_blitN; ++n){
                remainderTail += buf[(bufPos+n)%c_blitN];
            }
            float remainder = cumSum + remainderTail;

            float pulse = target - remainder; // pulse that takes us to -1~

            if(pulse){
                c_blitTable.getCoefficients(syncFraction, &blit[0], c_blitN);
                for(int n=0; n<c_blitN; ++n){
                    buf[(bufPos+n)%c_blitN] += pulse*blit[n];
                }
            }
            stage = 0;
        }
        syncPhase -= 2.f;
        if(resetPhase){
            internalPhase = syncPhase;
        }
    }

    last_cumSum = cumSum; // x
    cumSum = cumSum*leak + buf[bufPos] + (1.f-shape)*nFreq; // x+1

    float fc = freq*0.125f;

    outputs[0].value = CalcRcHp(cumSum, last_cumSum, outputs[0].value, fc, (float)fs);
    buf[bufPos] = 0.f;
    bufPos++;
    bufPos %= c_blitN;
}
