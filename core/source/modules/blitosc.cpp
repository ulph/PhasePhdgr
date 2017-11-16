#include "blitosc.hpp"
#include <string.h>
#include "sinc.hpp"
#include "inlines.hpp"
#include "rlc.hpp"

const auto c_blitTable = FractionalSincTable<BlitOsc::c_blitN>();

BlitOsc::BlitOsc()
    : buf{0.f}
    , bufPos(0)
    , cumSum(0.f)
    , stage(0)
    , internalPhase(0.f)
    , internalSyncPhase(0.f)
    , last_cumSum(0.f)
{
    inputs.push_back(Pad("freq"));
    inputs.push_back(Pad("shape")); // saw <-> square
    inputs.push_back(Pad("pwm"));
    inputs.push_back(Pad("syncFreq")); // 'master' osc freq, for osc sync purposes
    inputs.push_back(Pad("sync")); // how much to sync -- TODO non-linear map input range
    inputs.push_back(Pad("reset")); // reset both internal phases ... not suitable for osc sync as it'll alias
    inputs.push_back(Pad("offset", -1.f));
    outputs.push_back(Pad("output"));
}

void BlitOsc::blitOnePulse(float fraction, float multiplier) {
    float *sincPtr = nullptr;
    auto ret = c_blitTable.getCoefficientTablePointer(fraction, &sincPtr, c_blitN);
    assert(ret == c_blitN);
    assert(sincPtr != nullptr);
    for (int n = 0; n<c_blitN; ++n) {
        buf[(bufPos + n) % c_blitN] += multiplier*sincPtr[n];
    }
}

void BlitOsc::syncOnAuxPhase(float& phase, float& syncPhase, float syncAmount, float syncNFreq, float nFreq, float shape) {
    if (syncPhase > 1.f) {
        bool resetPhase = false;
        if (phase >= syncAmount) {
            resetPhase = true;
            float interval = (1.f - (syncPhase - syncNFreq));
            float syncFraction = interval / syncNFreq;

            float freqFractionCorrection = (syncNFreq / nFreq); // this helps a tiny bit extra, need to do the math as of why :P
            float sawCorrection = (1.f - shape)*nFreq*(1.f - syncFraction*freqFractionCorrection); // adjust for saw shape

            float target = -1.0f + sawCorrection; // target value

            float remainderTail = 0.f;
            for (int n = 0; n<c_blitN; ++n) {
                remainderTail += buf[(bufPos + n) % c_blitN];
            }
            float remainder = cumSum + remainderTail;

            float pulse = target - remainder; // pulse that takes us to -1~

            if (pulse) {
                blitOnePulse(syncFraction, pulse);
            }
            stage = 0;
        }
        syncPhase -= 2.f;
        if (resetPhase) {
            phase = syncPhase;
        }
    }
}

void BlitOsc::blitForward(float& phase, float nFreq, float shape, float pwm) {
    while (true) {
        if (stage == 0) {
            if (phase <= pwm) break;
            float interval = (pwm - (phase - nFreq));
            // deal with modulated pwm (not exactly correct but good enough)
            while (interval > 1.f) interval -= nFreq;
            while (interval < 0.f) interval += nFreq;
            float fraction = interval / nFreq;
            blitOnePulse(fraction, 2.f*shape);
            stage = 1;
        }
        if (stage == 1) {
            if (phase <= 1.0f) break;
            float interval = (1.f - (phase - nFreq));
            float fraction = interval / nFreq;
            blitOnePulse(fraction, -2.f);
            stage = 0;
            phase -= 2.0f;
        }
    }
}

void BlitOsc::incrementClocks(float nFreq, float syncNFreq) {
    internalSyncPhase += syncNFreq;
    internalPhase += nFreq;
}

void BlitOsc::integrateBuffer(uint32_t fs, float nFreq, float shape, float freq) {
    float prop_leak = nFreq * 0.01f;
    float leak = 1.f - prop_leak;

    float fc = freq*0.125f;

    last_cumSum = cumSum; // x
    cumSum = cumSum*leak + buf[bufPos] + (1.f - shape)*nFreq; // x+1

    outputs[0].value = CalcRcHp(cumSum, last_cumSum, outputs[0].value, fc, (float)fs);
    buf[bufPos] = 0.f;
    bufPos++;
    bufPos %= c_blitN;
}

void BlitOsc::resetOnSignal(float resetSignal) {
    // reset the clocks on an upflank through zero

    // TODO, either re-introduce the derived syncFreq or 
    // do something sort of ok without it, 
    // so that one can do sync using the reset signal

    if (resetSignal > 0.f && last_resetSignal <= 0.f) {
        internalSyncPhase = inputs[6].value;
        internalPhase = inputs[6].value;
    }
    last_resetSignal = resetSignal;
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

    resetOnSignal(inputs[5].value);

    incrementClocks(nFreq, syncNFreq);

    blitForward(internalPhase, nFreq, shape, pwm);

    syncOnAuxPhase(internalPhase, internalSyncPhase, syncAmount, syncNFreq, nFreq, shape);

    integrateBuffer(fs, nFreq, shape, freq);

}
