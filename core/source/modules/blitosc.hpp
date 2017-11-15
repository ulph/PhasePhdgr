#ifndef BLEPOSC_HPP
#define BLEPOSC_HPP

#include "module.hpp"
#include <assert.h>

class BlitOsc : public ModuleCRTP<BlitOsc>
{
public:
    static const int c_blitN = 16; // TODO, find a sweet spot or at least use min-phase sinc
private:
    float buf[c_blitN];
    int bufPos;
    float cumSum;
    int stage;
    float internalSyncPhase;
    float internalPhase;
    float last_cumSum; // for hp
    float last_resetSignal;
    void resetOnSignal(float resetSignal);
    void incrementClocks(float nFreq, float syncNFreq);
    void blitOnePulse(float fraction, float multiplier);
    void blitForward(float& phase, float nFreq, float shape, float pwm);
    void integrateBuffer(uint32_t fs, float nFreq, float shape, float freq);
    void syncOnAuxPhase(float& phase, float& syncPhase, float syncAmount, float syncNFreq, float nFreq, float shape);
public:
    BlitOsc();
    void process(uint32_t fs);
    static Module* factory() { return new BlitOsc(); }
};

#endif
