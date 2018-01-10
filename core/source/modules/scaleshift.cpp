#include "scaleshift.hpp"


ScaleShift::ScaleShift() {
    inputs.push_back(Pad("input"));
    inputs.push_back(Pad("scale", 2.f));
    inputs.push_back(Pad("shift", -1.f));
    outputs.push_back(Pad("output"));
}

void ScaleShift::process() {
    auto a = inputs[0].value;
    auto b = inputs[1].value;
    auto c = inputs[2].value;
    outputs[0].value = a * b + c;
}

void ScaleShift::block_process() {
    for (int i = 0; i < Pad::k_blockSize; ++i) {
        auto a = inputs[0].values[i];
        auto b = inputs[1].values[i];
        auto c = inputs[2].values[i];
        outputs[0].values[i] = a * b + c;
    }
}


ShiftScale::ShiftScale() {
    inputs.push_back(Pad("input"));
    inputs.push_back(Pad("shift", 1.f));
    inputs.push_back(Pad("scale", 0.5f));
    outputs.push_back(Pad("output"));
}

void ShiftScale::process() {
    auto a = inputs[0].value;
    auto b = inputs[1].value;
    auto c = inputs[2].value;

    outputs[0].value = (a + b) * c;
}

void ShiftScale::block_process() {
    for (int i = 0; i < Pad::k_blockSize; ++i) {
        auto a = inputs[0].values[i];
        auto b = inputs[1].values[i];
        auto c = inputs[2].values[i];
        outputs[0].values[i] = (a + b) * c;
    }
}


ScaleShiftMul::ScaleShiftMul() {
    inputs.push_back(Pad("input"));
    inputs.push_back(Pad("scale", 1.f));
    inputs.push_back(Pad("shift", 0.f));
    inputs.push_back(Pad("mul", 1.f));

    outputs.push_back(Pad("output"));
}

void ScaleShiftMul::process() {
    auto a = inputs[0].value;
    auto b = inputs[1].value;
    auto c = inputs[2].value;
    auto d = inputs[3].value;

    outputs[0].value = (a * b + c) * d;
}

void ScaleShiftMul::block_process() {
    for (int i = 0; i < Pad::k_blockSize; ++i) {
        auto a = inputs[0].values[i];
        auto b = inputs[1].values[i];
        auto c = inputs[2].values[i];
        auto d = inputs[3].values[i];
        outputs[0].values[i] = (a * b + c) * d;
    }
}