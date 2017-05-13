#include "bleposc.hpp"
#include <string.h>
#include "sinc.hpp"

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

const int c_blepNumFraction = 1000;
const auto c_blepTable = BlepTable(PolyBlepOsc::c_blepN, c_blepNumFraction, (float)M_PI);

PolyBlepOsc::PolyBlepOsc()
    : buf{0.f}
    , bufPos(0)
    , internalPhase(0.f)
    , widthDelay(0.5f)
    , pulseStage(0)
    , oldPhase(0)
{
    inputs.push_back(Pad("phase"));
    inputs.push_back(Pad("pwm", 0.5f));
    inputs.push_back(Pad("shape", 0.5f));
    outputs.push_back(Pad("output"));
}

void PolyBlepOsc::process(uint32_t fs)
{
    // adapted from Teemus bare bones polyblep example on KVR
    // http://www.kvraudio.com/forum/viewtopic.php?t=398553
    float newPhase = inputs[0].value;
    float pulseWidth = inputs[1].value;
    float mix = inputs[2].value;
    float freq = newPhase - oldPhase;
    freq = (freq >= 0 ? freq : freq + 2);
    freq *= 0.5;
    oldPhase = newPhase;
    int bufPosN = (bufPos+1)%c_blepN;

    internalPhase += freq;

    if(pulseWidth > 1) pulseWidth = 1;
    if(pulseWidth < 0) pulseWidth = 0;

    while(true)
    {
        if(pulseStage == 0)
        {
            if(internalPhase < pulseWidth) break;
            float t = (internalPhase - pulseWidth) / (widthDelay - pulseWidth + freq);
            buf[bufPos]             += mix * poly3blep0(t);
            buf[bufPosN] += mix * poly3blep1(t);
            pulseStage = 1;
        }
        if(pulseStage == 1)
        {
            if(internalPhase < 1) break;
            float t = (internalPhase - 1) / freq;
            buf[bufPos]             -= poly3blep0(t);
            buf[bufPosN] -= poly3blep1(t);
            pulseStage = 0;
            internalPhase -= 1;
        }
    }

    buf[bufPosN] += (1-mix)*internalPhase + mix*(pulseStage ? 1.f : 0.f);
    widthDelay = pulseWidth;

    outputs[0].value = (2*buf[bufPos] - 1);
    buf[bufPos] = 0;
    bufPos = bufPosN;
}
