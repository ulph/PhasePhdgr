#include "logic.hpp"
#include "inlines.hpp"

Threshold::Threshold() {
    inputs.push_back(Pad("input"));
    inputs.push_back(Pad("threshold", 0.5f));
    outputs.push_back(Pad("binary"));
}

void Threshold::process() {
    outputs[0].value = inputs[0].value >= inputs[1].value;
}

Counter::Counter() {
    inputs.push_back(Pad("trigger"));
    inputs.push_back(Pad("reset"));
    outputs.push_back(Pad("counter"));
}

void Counter::process() {
    auto trigger = inputs[0].value;
    auto reset = inputs[0].value;
    if(isHigh(reset) && !isHigh(lastReset)) counter = 0.0f;
    if(isHigh(trigger) && !isHigh(lastTrigger)) counter += 1.0f;
    lastTrigger = trigger;
    lastReset = reset;
    outputs[0].value = counter;
}
