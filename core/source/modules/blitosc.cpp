#include "blitosc.hpp"
#include <string.h>
#include "sinc.hpp"
#include "limits.hpp"
#include "rlc.hpp"

const auto c_blitTable = FractionalSincTable<BlitOsc::c_blitN>();

BlitOsc::BlitOsc()
{
    inputs.push_back(Pad("freq", "hz"));
    inputs.push_back(Pad("shape")); // saw <-> square
    inputs.push_back(Pad("pwm"));
    inputs.push_back(Pad("syncFreq", "hz")); // 'master' osc freq, for osc sync purposes
    inputs.push_back(Pad("sync")); // how much to sync -- TODO non-linear map input range
    inputs.push_back(Pad("reset")); // reset both internal phases ... not suitable for osc sync as it'll alias
    inputs.push_back(Pad("softReset"));
    inputs.push_back(Pad("dcRemoval", 0.125f));
    outputs.push_back(Pad("derivative"));
    outputs.push_back(Pad("out"));
    outputs.push_back(Pad("integral"));
}

inline void BlitOsc::blitOnePulse(double fraction, double multiplier) {
    double *sincPtr = nullptr;
    auto ret = c_blitTable.getCoefficientTablePointer(fraction, &sincPtr, c_blitN);
    assert(ret == c_blitN);
    assert(sincPtr != nullptr);
    for (int n = 0; n<c_blitN; ++n) {
        buf[(bufPos + n) % c_blitN] += multiplier*sincPtr[n];
    }
}

inline void BlitOsc::syncPhase(double& slavePhase, double& masterPhase, double syncAmount, double masterNFreq, double slaveNFreq, double shape) {
    if (masterNFreq <= 0) return;
    if (masterPhase > 1.0) {
        if (slavePhase > syncAmount) {
            float interval = (1.0 - (masterPhase - masterNFreq));
            // deal with modulated (estimated) masterNFreq
            while (interval > 1.0) interval -= masterNFreq;
            while (interval < 0.0) interval += masterNFreq;
            float syncFraction = interval / masterNFreq;
            float phaseInc = slaveNFreq * (masterPhase - 1.0) / masterNFreq;
            float sawCorrection = (1.0 - shape) * phaseInc;
            slavePhase = -1.0 + phaseInc;
            float target = -1.0 + sawCorrection; // target value

            float remainderTail = 0.0;
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
        masterPhase -= 2.0;
    }
}

inline void BlitOsc::blitForward(double& phase, double nFreq, double shape, double pwm) {
    while (true) {
        if (stage == 0) {
            if (phase <= pwm) break;
            float interval = (pwm - (phase - nFreq));
            // deal with modulated pwm (not exactly correct but good enough)
            while (interval > 1.0) interval -= nFreq;
            while (interval < 0.0) interval += nFreq;
            float fraction = interval / nFreq;
            blitOnePulse(fraction, 2.0*shape);
            stage = 1;
        }
        if (stage == 1) {
            if (phase <= 1.0) break;
            float interval = (1.0 - (phase - nFreq));
            float fraction = interval / nFreq;
            blitOnePulse(fraction, -2.0);
            stage = 0;
            phase -= 2.0;
        }
    }
}

inline void BlitOsc::incrementClocks(double nFreq, double syncNFreq) {
    internalSyncPhase += syncNFreq;
    internalPhase += nFreq;
}

inline void BlitOsc::integrateAndStore(double nFreq, double shape, double freq, double dcRemoval) {
    float prop_leak = nFreq * 0.01;
    float leak = 1.0 - prop_leak;

    float value = buf[bufPos] + (1.0 - shape)*nFreq;
    outputs[0].value = value;

    last_cumSum = cumSum;
    cumSum = cumSum*leak + value;
    outputs[1].value = CalcRcHp(cumSum, last_cumSum, outputs[1].value, freq*dcRemoval, fsInv);

    last_cumCumSum = cumCumSum;
    cumCumSum = cumCumSum*leak + (2 + 2*(1-shape))*nFreq*outputs[1].value;
    outputs[2].value = CalcRcHp(cumCumSum, last_cumCumSum, outputs[2].value, freq*0.125, fsInv);

    buf[bufPos] = 0.0;
    bufPos++;
    bufPos %= c_blitN;
}

inline void BlitOsc::hardResetOnSignal(double resetSignal) {
    if (resetSignal > 0.0 && last_resetSignal <= 0.0) {
        internalSyncPhase = -1.0;
        internalPhase = -1.0;
        cumSum = -1.0;
        cumCumSum = 0.0;
        last_cumSum = -1.0;
        last_cumCumSum = 0.0;
        outputs[1].value = -1.0;
        outputs[2].value = 0.0;
        for(int i=0; i<c_blitN; i++) buf[i] = 0.0;
        bufPos = 0;
        stage = 0;
    }
    last_resetSignal = resetSignal;
}

inline void BlitOsc::softResetOnSignal(double resetSignal, double syncAmount, double nFreq, double shape) {
    if (resetSignal > 0.0 && last_softResetSignal <= 0.0) {
        double mockSyncPhase = 1.0 + resetSignal;
        double mockSyncNFreq = resetSignal - last_softResetSignal;
        syncPhase(internalPhase, mockSyncPhase, syncAmount, mockSyncNFreq, nFreq, shape);
    }
    last_softResetSignal = resetSignal;
}

void BlitOsc::process()
{
    double freq = limit(inputs[0].value, 1.0, fs*0.5);
    double shape = limit(inputs[1].value, 0.0f, 1.0);
    double pwm = limit(inputs[2].value);
    double syncFreq = inputs[3].value;
    double syncAmount = 2.0*(1.0-limit(inputs[4].value, 0.0, 1.0)) - 1.0;

    double dcRemoval = limitLow(inputs[7].value, 0.0078125);

    double nFreq = 2.0*freq * fsInv;
    double syncNFreq = 2.0*syncFreq * fsInv;

    if(nFreq == 0) return; // nothing to do, just exit

    hardResetOnSignal(inputs[5].value);

    softResetOnSignal(inputs[6].value, syncAmount, nFreq, shape);

    incrementClocks(nFreq, syncNFreq);

    syncPhase(internalPhase, internalSyncPhase, syncAmount, syncNFreq, nFreq, shape);

    blitForward(internalPhase, nFreq, shape, pwm);

    integrateAndStore(nFreq, shape, freq, dcRemoval);

}
