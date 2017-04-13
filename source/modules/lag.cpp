#include "lag.hpp"

Lag::Lag()
{
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("amount", 0.9f));
    outputs.push_back(Pad("out"));
}

void Lag::process(uint32_t fs)
{
    outputs[0].value = outputs[0].value * inputs[1].value + inputs[0].value * ( 1.0f - inputs[1].value );
}
