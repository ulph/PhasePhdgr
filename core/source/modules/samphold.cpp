#include "samphold.hpp"

SampleAndHold::SampleAndHold() :
    lastTrigger(0.f),
    heldValue(0.f)
{
    inputs.push_back(Pad("sample"));
    inputs.push_back(Pad("trigger"));
    outputs.push_back(Pad("value"));
}

void SampleAndHold::process(uint32_t fs)
{
    float value = inputs[0].value;
    float trigger = inputs[1].value;
    if(lastTrigger <= 0.f && trigger > 0.f){
        heldValue = value;
    }
    lastTrigger = trigger;
    outputs[0].value = heldValue;
}

// TODO, docstring
