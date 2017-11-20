#include "logic.hpp"

Threshold::Threshold()
{
    inputs.push_back(Pad("input"));
    inputs.push_back(Pad("threshold", 0.f));
    outputs.push_back(Pad("binary"));
}

void Threshold::process(uint32_t fs) {
    outputs[0].value = inputs[0].value >= inputs[1].value;
}
