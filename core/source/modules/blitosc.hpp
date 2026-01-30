#ifndef BLEPOSC_HPP
#define BLEPOSC_HPP

#include "module.hpp"
#include <assert.h>

class BlitOsc : public ModuleCRTP<BlitOsc>
{
public:
    static const int c_blitN = 15; // TODO, find a sweet spot or at least use min-phase sinc
private:
    double buf[c_blitN] = { 0.f };
    int bufPos = 0;
    double cumSum = 0.0f;
    double cumCumSum = 0.0f;
    int stage = 0;
    double internalSyncPhase = 0.0f;
    double internalPhase = 0.0f;
    double last_cumSum = 0.0f;
    double last_cumCumSum = 0.0f;
    double last_resetSignal = 0.0f;
    double last_softResetSignal = 0.0f;
    inline void hardResetOnSignal(double resetSignal);
    inline void softResetOnSignal(double resetSignal, double syncAmount, double nFreq, double shape);
    inline void incrementClocks(double nFreq, double syncNFreq);
    inline void blitOnePulse(double fraction, double multiplier);
    inline void blitForward(double& phase, double nFreq, double shape, double pwm);
    inline void integrateAndStore(double nFreq, double shape, double freq, double dcRemoval);
    inline void syncPhase(double& phase, double& syncPhase, double syncAmount, double syncNFreq, double nFreq, double shape);
public:
    BlitOsc();
    void process();
    static Module* factory() { return new BlitOsc(); }
};

#endif
