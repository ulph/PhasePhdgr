#include "tanh.hpp"

TanH::TanH() {
    inputs.push_back(Pad("in"));
    outputs.push_back(Pad("tanh"));
}

void TanH::process(uint32_t fs) {
    outputs[0].value = tanhf(inputs[0].value);
}
