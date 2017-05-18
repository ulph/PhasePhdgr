#ifndef BLEPOSC_HPP
#define BLEPOSC_HPP

#include "module.hpp"
#include <assert.h>

class BlitOsc : public Module
{
public:
    static const int c_blitN = 16; // for the derivative
    static const int c_dBlitN =16; // for the bias derivative (2nd order derivative strictly but we seperate the concerns here)
private:
    float buf[c_blitN];
    float blit[c_blitN];
    int bufPos;
    float cumSum;
    int stage;
    float sync;
    float internalPhase;
    float last_nFreq;
    float last_pwm;
    float last_shape;
    float last_cumSum;
    float bias;
    float dBuf[c_dBlitN];
    float dBlit[c_dBlitN];
    int dBufPos;
public:
    BlitOsc();
    void process(uint32_t fs);
    static Module* factory() { return new BlitOsc(); }
};

#endif
