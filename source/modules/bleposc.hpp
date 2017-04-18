#ifndef BLEPOSC_HPP
#define BLEPOSC_HPP

#include "module.hpp"

static inline float poly3blep0(float t)
{
    if(t < 0) return 0;
    if(t > 1) return 1;

    float t2 = t*t;
    return t * t2 - 0.5f * t2 * t2;
}

static inline float poly3blep1(float t)
{
    return -poly3blep0(1-t);
}

class PolyBlepOsc : public Module
{
private:
    float phase;
    float blepDelay;
    float widthDelay;
    int   pulseStage;
public:
    PolyBlepOsc();
    void process(uint32_t fs);
    static Module* factory() { return new PolyBlepOsc(); }
};

#endif
