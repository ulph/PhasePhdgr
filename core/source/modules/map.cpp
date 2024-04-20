#include "map.hpp"
#include <assert.h>
#include "inlines.hpp"

Clamp::Clamp()
{
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("low", 0.f));
    inputs.push_back(Pad("high", 1.f));
    outputs.push_back(Pad("out"));
}

void Clamp::processSample(int sample)
{
    auto v = inputs[0].values[sample];
    auto lo = inputs[1].values[sample];
    auto hi = inputs[2].values[sample];
    outputs[0].values[sample] = limit(v, lo, hi);
}

RangeMap::RangeMap() {
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("inA", 0.0f));
    inputs.push_back(Pad("inB", 1.0f));
    inputs.push_back(Pad("outA", 1.0f));
    inputs.push_back(Pad("outB", 0.0f));
    outputs.push_back(Pad("out"));
}

void RangeMap::processSample(int sample) {
    float x = inputs[0].values[sample];
    float inA = inputs[1].values[sample];
    float inB = inputs[2].values[sample];
    float outA = inputs[3].values[sample];
    float outB = inputs[4].values[sample];
    float y = outA + ((outB - outA) / (inB - inA)) * (x - inA);
    outputs[0].values[sample] = y;
}

ScaleShift::ScaleShift() {
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("mul", 2.f));
    inputs.push_back(Pad("add", -1.f));
    outputs.push_back(Pad("out"));
}

void ScaleShift::processSample(int sample) {
    auto a = inputs[0].values[sample];
    auto b = inputs[1].values[sample];
    auto c = inputs[2].values[sample];
    outputs[0].values[sample] = a * b + c;
}

ClampInv::ClampInv() {
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("low", 0.0f));
    inputs.push_back(Pad("high", 1.0f));
    outputs.push_back(Pad("out"));
    
}

void ClampInv::processSample(int sample) {
    float v = inputs[0].values[sample];
    float lo = inputs[1].values[sample];
    float hi = inputs[2].values[sample];
    if (hi >= lo) {
        v = limit(v, lo, hi); // clamp
        outputs[0].values[sample] = hi - (v - lo); // invert
    }
    else {
        // nonsensical values
        outputs[0].values[sample] = v;
    }
}
