#include "blitosc.hpp"
#include <string.h>
#include "sinc.hpp"
#include "inlines.hpp"
#include "rlc.hpp"

const auto c_blitTable = FractionalSincTable<BlitOsc::c_blitN>();

BlitOsc::BlitOsc()
{
    inputs.push_back(Pad("freq"));
    inputs.push_back(Pad("shape")); // saw <-> square
    inputs.push_back(Pad("pwm"));
    inputs.push_back(Pad("syncFreq")); // 'master' osc freq, for osc sync purposes
    inputs.push_back(Pad("sync")); // how much to sync -- TODO non-linear map input range
    inputs.push_back(Pad("reset")); // reset both internal phases ... not suitable for osc sync as it'll alias
    inputs.push_back(Pad("offset", -1.f));
    outputs.push_back(Pad("derivative"));
    outputs.push_back(Pad("output"));
    outputs.push_back(Pad("integral"));
}

inline void BlitOsc::blitOnePulse(float fraction, float multiplier) {
    float *sincPtr = nullptr;
    auto ret = c_blitTable.getCoefficientTablePointer(fraction, &sincPtr, c_blitN);
    assert(ret == c_blitN);
    assert(sincPtr != nullptr);
    for (int n = 0; n<c_blitN; ++n) {
        buf[(bufPos + n) % c_blitN] += multiplier*sincPtr[n];
    }
}

inline void BlitOsc::syncOnAuxPhase(float& phase, float& syncPhase, float syncAmount, float syncNFreq, float nFreq, float shape) {
    if (syncPhase > 1.f) {
        if (phase > syncAmount) {
            float interval = (1.f - (syncPhase - syncNFreq));
            float syncFraction = interval / syncNFreq;
            float phaseInc = nFreq * (syncPhase - 1.0f)/syncNFreq;
            float sawCorrection = (1.f - shape) * phaseInc;
            phase = -1.0 + phaseInc;
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
    }
}

inline void BlitOsc::blitForward(float& phase, float nFreq, float shape, float pwm) {
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

inline void BlitOsc::incrementClocks(float nFreq, float syncNFreq) {
    internalSyncPhase += syncNFreq;
    internalPhase += nFreq;
}

inline void BlitOsc::integrateBuffer(float nFreq, float shape, float freq) {
    float prop_leak = nFreq * 0.01f;
    float leak = 1.f - prop_leak;

    float value = buf[bufPos] + (1.f - shape)*nFreq;
    outputs[0].value = value;

    last_cumSum = cumSum;
    cumSum = cumSum*leak + value;
    outputs[1].value = CalcRcHp(cumSum, last_cumSum, outputs[1].value, freq*0.125f, fsInv);

    last_cumCumSum = cumCumSum;
    cumCumSum = cumCumSum*leak + 2*nFreq*outputs[1].value;
    outputs[2].value = CalcRcHp(cumCumSum, last_cumCumSum, outputs[2].value, freq*0.125f, fsInv);

    buf[bufPos] = 0.f;
    bufPos++;
    bufPos %= c_blitN;
}

inline void BlitOsc::resetOnSignal(float resetSignal) {
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

void BlitOsc::process()
{
    float freq = limit(inputs[0].value, 1.f, fs*0.5f);
    float shape = limit(inputs[1].value, 0.0f, 1.0f);
    float pwm = limit(inputs[2].value);
    float syncFreq = inputs[3].value;
    float syncAmount = 2.f*(1.f-limit(inputs[4].value, 0.f, 1.f)) - 1.f;

    float nFreq = 2.f*freq * fsInv; // TODO get this from inBus wall directly
    float syncNFreq = 2.f*syncFreq * fsInv; // TODO get this from inBus wall directly

    if(nFreq == 0) return; // nothing to do, just exit

    resetOnSignal(inputs[5].value);

    incrementClocks(nFreq, syncNFreq);

    blitForward(internalPhase, nFreq, shape, pwm);

    syncOnAuxPhase(internalPhase, internalSyncPhase, syncAmount, syncNFreq, nFreq, shape);

    integrateBuffer(nFreq, shape, freq);

}
