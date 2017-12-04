#include "sympow.hpp"

SymPow::SymPow() {
    inputs.push_back(Pad("base", 1.0));
    inputs.push_back(Pad("exp", 1.0));
    outputs.push_back(Pad("pow"));
}

void SymPow::process() {
    float v = fabs(inputs[0].value);
    float sign = inputs[0].value >= 0 ? 1.f : -1.f;
    outputs[0].value = sign*powf(v, inputs[1].value);
}
