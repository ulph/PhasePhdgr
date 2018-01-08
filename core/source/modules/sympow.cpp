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

SymLog2::SymLog2() {
    inputs.push_back(Pad("value", 1.0));
    outputs.push_back(Pad("log"));
}

void SymLog2::process() {
    float v = fabs(inputs[0].value);
    float sign = inputs[0].value >= 0 ? 1.f : -1.f;
    outputs[0].value = sign*logf(v) / logf(2.0f);
}
