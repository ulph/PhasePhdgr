#include "bleposc.hpp"
#include <string.h>

PolyBlepOsc::PolyBlepOsc()
    : phase(0.f)
    , blepDelay(0.f)
    , widthDelay(0.5f)
    , pulseStage(0)
{
    inputs.push_back(Pad("freq", 440.0f));
    inputs.push_back(Pad("pwm", 0.5f));
    inputs.push_back(Pad("shape", 0.0f));
    outputs.push_back(Pad("output"));
}

void PolyBlepOsc::process(uint32_t fs)
{
    // adapted from Teemus bare bones polyblep example on KVR
    // http://www.kvraudio.com/forum/viewtopic.php?t=398553

    float mix = inputs[2].value;
    float out = blepDelay;
    blepDelay = 0;
    float freq = inputs[0].value;
    phase += freq/((float)fs);

    float pulseWidth = 0.5+0.5*inputs[1].value;

    if(pulseWidth > 1) pulseWidth = 1;
    if(pulseWidth < 0) pulseWidth = 0;

    while(true)
    {
        if(pulseStage == 0)
        {
            if(phase < pulseWidth) break;
            float t = (phase - pulseWidth) / (widthDelay - pulseWidth + freq);
            out       += mix * poly3blep0(t);
            blepDelay += mix * poly3blep1(t);
            pulseStage = 1;
        }

        if(pulseStage == 1)
        {
            if(phase < 1) break;
            float t = (phase - 1) / freq;

            out       -= poly3blep0(t);
            blepDelay -= poly3blep1(t);

            pulseStage = 0;
            phase -= 1;
        }
    }

    blepDelay += (1-mix)*phase + mix*(pulseStage ? 1.f : 0.f);

    outputs[0].value = (2*out - 1);

    widthDelay = pulseWidth;

}
