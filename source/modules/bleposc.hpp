#ifndef BLEPOSC_HPP
#define BLEPOSC_HPP

#include "module.hpp"
#include <assert.h>

class PolyBlepOsc : public Module
{
public:
    static const int c_blepN = 16;
private:
    float buf[c_blepN];
    float blep[c_blepN];
    int bufPos;
    float internalPhase;
    float widthDelay;
    int   pulseStage;
    float oldPhase;
public:
    PolyBlepOsc();
    void process(uint32_t fs);
    static Module* factory() { return new PolyBlepOsc(); }
};

#endif
