#include "map.hpp"

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
    if(v < lo) v = lo;
    else if(v > hi) v = hi;
    outputs[0].value = v;
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
    float v = inputs[0].value;
    // TODO; implement
}
