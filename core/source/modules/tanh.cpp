#include "tanh.hpp"

TanH::TanH() {
    inputs.push_back(Pad("in"));
    outputs.push_back(Pad("tanh"));
}

void TanH::process() {
    outputs[0].value = tanhf(inputs[0].value);
}

NormalizedTanH::NormalizedTanH() {
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("prescaler", 1.0f));
    outputs.push_back(Pad("tanh"));
}

void NormalizedTanH::process() {
    float s = inputs[1].value;
    float f = 1.0f / tanhf(s);
    outputs[0].value = tanhf(inputs[0].value * s) * f;
}
