#include "sympow.hpp"
#include <math.h>

SymPow::SymPow() {
    inputs.push_back(Pad("base", 1.0));
    inputs.push_back(Pad("exp", 1.0));
    outputs.push_back(Pad("out"));
}

void SymPow::processSample(int sample) {
    float v = fabs(inputs[0].values[sample]);
    float sign = inputs[0].values[sample] >= 0 ? 1.f : -1.f;
    outputs[0].values[sample] = sign*powf(v, inputs[1].values[sample]);
}

SymLog2::SymLog2() {
    inputs.push_back(Pad("in", 1.0));
    outputs.push_back(Pad("out"));
}

void SymLog2::processSample(int sample) {
    float v = fabs(inputs[0].values[sample]);
    float sign = inputs[0].values[sample] >= 0 ? 1.f : -1.f;
    outputs[0].values[sample] = sign*logf(v) / logf(2.0f);
}
