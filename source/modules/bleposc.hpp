#ifndef BLEPOSC_HPP
#define BLEPOSC_HPP

#include "module.hpp"
#include <assert.h>

static inline float poly3blep0(float t)
{
    assert(t >= 0 && t<= 1);
    if(t < 0) return 0;
    if(t > 1) return 1;
//    return t*t*t - 0.5*t*t*t*t;
    return 0.5f*t*t;
}

static inline float poly3blep1(float t)
{
    return -poly3blep0(1-t);
}

class PolyBlepOsc : public Module
{
private:
    float internalPhase;
    float blepDelay;
    float widthDelay;
    int   pulseStage;
    float oldPhase;
public:
    PolyBlepOsc();
    void process(uint32_t fs);
    static Module* factory() { return new PolyBlepOsc(); }
};

#endif
