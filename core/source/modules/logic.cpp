#include "logic.hpp"
#include "inlines.hpp"

Threshold::Threshold() {
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("threshold", 0.5f));
    outputs.push_back(Pad("out"));
}

void Threshold::processSample(int sample) {
    outputs[0].values[sample] = inputs[0].values[sample] >= inputs[1].values[sample];
}

Counter::Counter() {
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("reset"));
    outputs.push_back(Pad("out"));
}

void Counter::processSample(int sample) {
    auto trigger = inputs[0].values[sample];
    auto reset = inputs[1].values[sample];
    if(isHigh(reset) && !isHigh(lastReset)) counter = 0.0f;
    if(isHigh(trigger) && !isHigh(lastTrigger)) counter += 1.0f;
    lastTrigger = trigger;
    lastReset = reset;
    outputs[0].values[sample] = counter;
}
