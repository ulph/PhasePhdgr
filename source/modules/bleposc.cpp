#include "bleposc.hpp"
#include <string.h>

PolyBlepOsc::PolyBlepOsc()
    : internalPhase(0.f)
    , blepDelay(0.f)
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

    float out = blepDelay;
    blepDelay = 0;

    internalPhase += freq;

    if(pulseWidth > 1) pulseWidth = 1;
    if(pulseWidth < 0) pulseWidth = 0;

    while(true)
    {
        if(pulseStage == 0)
        {
            if(internalPhase < pulseWidth) break;
            float t = (internalPhase - pulseWidth) / (widthDelay - pulseWidth + freq);
            out       += mix * poly3blep0(t);
            blepDelay += mix * poly3blep1(t);
            pulseStage = 1;
        }
        if(pulseStage == 1)
        {
            if(internalPhase < 1) break;
            float t = (internalPhase - 1) / freq;
            out       -= poly3blep0(t);
            blepDelay -= poly3blep1(t);
            pulseStage = 0;
            internalPhase -= 1;
        }
    }

    blepDelay += (1-mix)*internalPhase + mix*(pulseStage ? 1.f : 0.f);
    widthDelay = pulseWidth;
    outputs[0].value = (2*out - 1);
}
