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
    inputs.push_back(Pad("softReset"));
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

inline void BlitOsc::syncPhase(float& slavePhase, float& masterPhase, float syncAmount, float masterNFreq, float slaveNFreq, float shape) {
    if (masterPhase > 1.0f) {
        if (slavePhase > syncAmount) {
            float interval = (1.0f - (masterPhase - masterNFreq));
            float syncFraction = interval / masterNFreq;
            float phaseInc = slaveNFreq * (masterPhase - 1.0f)/masterNFreq;
            float sawCorrection = (1.0f - shape) * phaseInc;
            slavePhase = -1.0f + phaseInc;
            float target = -1.0f + sawCorrection; // target value

            float remainderTail = 0.0f;
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
        masterPhase -= 2.f;
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
    cumCumSum = cumCumSum*leak + (2 + 2*(1-shape))*nFreq*outputs[1].value;
    outputs[2].value = CalcRcHp(cumCumSum, last_cumCumSum, outputs[2].value, freq*0.125f, fsInv);

    buf[bufPos] = 0.f;
    bufPos++;
    bufPos %= c_blitN;
}

inline void BlitOsc::resetOnSignal(float resetSignal) {
    if (resetSignal > 0.f && last_resetSignal <= 0.f) {
        internalSyncPhase = inputs[6].value;
        internalPhase = inputs[6].value;
        cumSum = 0.0f;
        cumCumSum = 0.0f;
        last_cumSum = 0.0f;
        last_cumCumSum = 0.0f;
        outputs[1].value = 0.0f;
        outputs[2].value = 0.0f;
        for(int i=0; i<c_blitN; i++) buf[i] = 0.0f;
        bufPos = 0;
        stage = 0;
    }
    last_resetSignal = resetSignal;
}

inline void BlitOsc::softResetOnSignal(float resetSignal, float syncAmount, float nFreq, float shape) {
    if (resetSignal > 0.f && last_softResetSignal <= 0.f) {
        float mockSyncPhase = 1.0f + resetSignal;
        float new_mockSyncNFreq = resetSignal / (resetSignal - last_softResetSignal);
        mockSyncNFreq = 0.5f*mockSyncNFreq + 0.5f*new_mockSyncNFreq;
        syncPhase(internalPhase, mockSyncPhase, syncAmount, mockSyncNFreq, nFreq, shape);
    }
    last_softResetSignal = resetSignal;
}

void BlitOsc::process()
{
    float freq = limit(inputs[0].value, 1.f, fs*0.5f);
    float shape = limit(inputs[1].value, 0.0f, 1.0f);
    float pwm = limit(inputs[2].value);
    float syncFreq = inputs[3].value;
    float syncAmount = 2.f*(1.f-limit(inputs[4].value, 0.f, 1.f)) - 1.f;

    float nFreq = 2.f*freq * fsInv;
    float syncNFreq = 2.f*syncFreq * fsInv;

    if(nFreq == 0) return; // nothing to do, just exit

    resetOnSignal(inputs[5].value);
    softResetOnSignal(inputs[7].value, syncAmount, nFreq, shape);

    incrementClocks(nFreq, syncNFreq);

    blitForward(internalPhase, nFreq, shape, pwm);

    syncPhase(internalPhase, internalSyncPhase, syncAmount, syncNFreq, nFreq, shape);

    integrateBuffer(nFreq, shape, freq);

}
