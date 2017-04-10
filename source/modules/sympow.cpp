#include "sympow.hpp"

SymPow::SymPow() {
    inputs.push_back(Pad("base", 1.0));
    inputs.push_back(Pad("exp", 1.0));
    outputs.push_back(Pad("pow"));
}

void SymPow::process(uint32_t fs) {
    float v = abs(inputs[0].value);
    float sign = inputs[0].value >= 0 ? 1 : -1;
    outputs[0].value = sign*powf(v, inputs[1].value);
}
