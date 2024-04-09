#include "samphold.hpp"

SampleAndHold::SampleAndHold() :
    lastTrigger(0.f),
    heldValue(0.f)
{
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("trigger"));
    outputs.push_back(Pad("out"));
}

void SampleAndHold::processSample(int sample)
{
    float value = inputs[0].values[sample];
    float trigger = inputs[1].values[sample];
    if(lastTrigger <= 0.f && trigger > 0.f){
        heldValue = value;
    }
    lastTrigger = trigger;
    outputs[0].values[sample] = heldValue;
}

// TODO, docstring
