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

void Clamp::process()
{
    auto v = inputs[0].value;
    auto lo = inputs[1].value;
    auto hi = inputs[2].value;
    outputs[0].value = limit(v, lo, hi);
}

RangeMap::RangeMap() {
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("inA", 0.0f));
    inputs.push_back(Pad("inB", 1.0f));
    inputs.push_back(Pad("outA", 1.0f));
    inputs.push_back(Pad("outB", 0.0f));
    outputs.push_back(Pad("out"));
}

void RangeMap::process() {
    float x = inputs[0].value;
    float inA = inputs[1].value;
    float inB = inputs[2].value;
    float outA = inputs[3].value;
    float outB = inputs[4].value;
    float y = outA + ((outB - outA) / (inB - inA)) * (x - inA);
    outputs[0].value = y;
}

ScaleShift::ScaleShift() {
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("mul", 2.f));
    inputs.push_back(Pad("add", -1.f));
    outputs.push_back(Pad("out"));
}

void ScaleShift::process() {
    auto a = inputs[0].value;
    auto b = inputs[1].value;
    auto c = inputs[2].value;
    outputs[0].value = a * b + c;    
}

ClampInv::ClampInv() {
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("low", 0.0f));
    inputs.push_back(Pad("high", 1.0f));
    outputs.push_back(Pad("out"));
    
}

void ClampInv::process() {
    float v = inputs[0].value;
    float lo = inputs[1].value;
    float hi = inputs[2].value;
    if (hi >= lo) {
        v = limit(v, lo, hi); // clamp
        outputs[0].value = hi - (v - lo); // invert
    }
    else {
        // nonsensical values
        outputs[0].value = v;
    }
}