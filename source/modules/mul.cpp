#include "mul.hpp"

Mul::Mul()
{
    inputs.push_back(Pad("in1"));
    inputs.push_back(Pad("in2"));
    outputs.push_back(Pad("prod"));
}

void Mul::process(uint32_t fs) {
    outputs[0].value = inputs[0].value * inputs[1].value;
}

Gain::Gain()
{
    inputs.push_back(Pad("left"));
    inputs.push_back(Pad("right"));
    inputs.push_back(Pad("gain"));
    outputs.push_back(Pad("left"));
    outputs.push_back(Pad("right"));
}

void Gain::process(uint32_t fs) {
    outputs[0].value = inputs[0].value * inputs[2].value;
    outputs[1].value = inputs[1].value * inputs[2].value;
}
