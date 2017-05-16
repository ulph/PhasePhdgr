#ifndef BLEPOSC_HPP
#define BLEPOSC_HPP

#include "module.hpp"
#include <assert.h>

class BlitOsc : public Module
{
public:
    static const int c_blitN = 32; // TODO, find a sweet spot or at least use min-phase sinc
private:
    float buf[c_blitN];
    float blit[c_blitN];
    int bufPos;
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
