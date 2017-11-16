#include "mul.hpp"

Mul::Mul()
{
    inputs.push_back(Pad("in1", 1.f));
    inputs.push_back(Pad("in2", 1.f));
    outputs.push_back(Pad("prod"));
}

void Mul::process(uint32_t fs) {
    outputs[0].value = inputs[0].value * inputs[1].value;
}

MulTri::MulTri()
{
    inputs.push_back(Pad("in1", 1.f));
    inputs.push_back(Pad("in2", 1.f));
    inputs.push_back(Pad("in3", 1.f));
    outputs.push_back(Pad("prod"));
}

void MulTri::process(uint32_t fs) {
    outputs[0].value = inputs[0].value * inputs[1].value * inputs[2].value;
}

MulQuad::MulQuad()
{
    inputs.push_back(Pad("in1", 1.f));
    inputs.push_back(Pad("in2", 1.f));
    inputs.push_back(Pad("in3", 1.f));
    inputs.push_back(Pad("in4", 1.f));
    outputs.push_back(Pad("prod"));
}

void MulQuad::process(uint32_t fs) {
    outputs[0].value = inputs[0].value * inputs[1].value * inputs[2].value * inputs[3].value;
}

Gain::Gain()
{
    inputs.push_back(Pad("left"));
    inputs.push_back(Pad("right"));
    inputs.push_back(Pad("gain", 1.f));
    outputs.push_back(Pad("left"));
    outputs.push_back(Pad("right"));
}

void Gain::process(uint32_t fs) {
    outputs[0].value = inputs[0].value * inputs[2].value;
    outputs[1].value = inputs[1].value * inputs[2].value;
}
