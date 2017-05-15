#ifndef BLEPOSC_HPP
#define BLEPOSC_HPP

#include "module.hpp"
#include <assert.h>

class BlitOsc : public Module
{
public:
    static const int c_blitN = 32;
private:
    float buf[c_blitN];
    float blit[c_blitN];
    int bufPos;
    float oldPhase;
    float cumSum;
    int stage;
    float sync;
    float internalPhase;
public:
    BlitOsc();
    void process(uint32_t fs);
    static Module* factory() { return new BlitOsc(); }
};

#endif
