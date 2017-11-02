#include "scaleshift.hpp"

ScaleShift::ScaleShift() {
    inputs.push_back(Pad("input"));
    inputs.push_back(Pad("scale", 2.f));
    inputs.push_back(Pad("shift", -1.f));
    outputs.push_back(Pad("output"));
}

void ScaleShift::process(uint32_t fs) {
    outputs[0].value = inputs[0].value * inputs[1].value + inputs[2].value;
}
