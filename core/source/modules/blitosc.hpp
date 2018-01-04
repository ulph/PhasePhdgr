#ifndef BLEPOSC_HPP
#define BLEPOSC_HPP

#include "module.hpp"
#include <assert.h>

class BlitOsc : public ModuleCRTP<BlitOsc>
{
public:
    static const int c_blitN = 16; // TODO, find a sweet spot or at least use min-phase sinc
private:
    float buf[c_blitN] = { 0.f };
    int bufPos = 0;
    float cumSum = 0.0f;
    float cumCumSum = 0.0f;
    int stage = 0;
    float internalSyncPhase = 0.0f;
    float internalPhase = 0.0f;
    float last_cumSum = 0.0f;
    float last_cumCumSum = 0.0f;
    float last_resetSignal = 0.0f;
    float last_softResetSignal = 0.0f;
    inline void hardResetOnSignal(float resetSignal);
    inline void softResetOnSignal(float resetSignal, float syncAmount, float nFreq, float shape);
    inline void incrementClocks(float nFreq, float syncNFreq);
    inline void blitOnePulse(float fraction, float multiplier);
    inline void blitForward(float& phase, float nFreq, float shape, float pwm);
    inline void integrateAndStore(float nFreq, float shape, float freq, float dcRemoval);
    inline void syncPhase(float& phase, float& syncPhase, float syncAmount, float syncNFreq, float nFreq, float shape);
public:
    BlitOsc();
    void process();
    static Module* factory() { return new BlitOsc(); }
};

#endif
