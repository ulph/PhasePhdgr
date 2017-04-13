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

RcLp::RcLp()
{
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("freq", 16000.f));
    outputs.push_back(Pad("out"));
}

void RcLp::process(uint32_t fs)
{
    float d = 2 * M_PI*inputs[1].value / (float) fs;
    float a = d / (d + 1);
    outputs[0].value = a*inputs[0].value + (1 - a)*outputs[0].value;
}

RcHp::RcHp() 
    : x(0.0f)
{
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("freq", 40.0f));
    outputs.push_back(Pad("out"));
}

void RcHp::process(uint32_t fs)
{
    float a = 1.0f / (2 * M_PI*inputs[1].value / (float)fs + 1);
    outputs[0].value = a*outputs[0].value + a*(inputs[0].value - x);
    x = inputs[0].value;
}
