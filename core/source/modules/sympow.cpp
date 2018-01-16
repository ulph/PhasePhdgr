#include "sympow.hpp"
#include <math.h>

SymPow::SymPow() {
    inputs.push_back(Pad("base", 1.0));
    inputs.push_back(Pad("exp", 1.0));
    outputs.push_back(Pad("out"));
}

void SymPow::process() {
    float v = fabs(inputs[0].value);
    float sign = inputs[0].value >= 0 ? 1.f : -1.f;
    outputs[0].value = sign*powf(v, inputs[1].value);
}

SymLog2::SymLog2() {
    inputs.push_back(Pad("in", 1.0));
    outputs.push_back(Pad("out"));
}

void SymLog2::process() {
    float v = fabs(inputs[0].value);
    float sign = inputs[0].value >= 0 ? 1.f : -1.f;
    outputs[0].value = sign*logf(v) / logf(2.0f);
}
