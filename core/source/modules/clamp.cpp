#include "clamp.hpp"

Clamp::Clamp()
{
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("low", 0.f));
    inputs.push_back(Pad("high", 1.f));
    outputs.push_back(Pad("clamp"));
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
