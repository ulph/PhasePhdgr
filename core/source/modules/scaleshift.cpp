#include "scaleshift.hpp"


ScaleShift::ScaleShift() {
    inputs.push_back(Pad("input"));
    inputs.push_back(Pad("scale", 2.f));
    inputs.push_back(Pad("shift", -1.f));
    outputs.push_back(Pad("output"));
}

void ScaleShift::process(uint32_t fs) {
    auto a = inputs[0].value;
    auto b = inputs[1].value;
    auto c = inputs[2].value;

    outputs[0].value = a * b + c;
}


ShiftScale::ShiftScale() {
    inputs.push_back(Pad("input"));
    inputs.push_back(Pad("shift", 1.f));
    inputs.push_back(Pad("scale", 0.5f));
    outputs.push_back(Pad("output"));
}

void ShiftScale::process(uint32_t fs) {
    auto a = inputs[0].value;
    auto b = inputs[1].value;
    auto c = inputs[2].value;

    outputs[0].value = (a + b) * c;
}


ScaleShiftMul::ScaleShiftMul() {
    inputs.push_back(Pad("input"));
    inputs.push_back(Pad("scale", 1.f));
    inputs.push_back(Pad("shift", 0.f));
    inputs.push_back(Pad("mul", 1.f));

    outputs.push_back(Pad("output"));
}

void ScaleShiftMul::process(uint32_t fs) {
    auto a = inputs[0].value;
    auto b = inputs[1].value;
    auto c = inputs[2].value;
    auto d = inputs[3].value;

    outputs[0].value = (a * b + c) * d;
}
