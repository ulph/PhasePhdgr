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

void Mul::block_process(uint32_t fs) {
    for (int i = 0; i < ConnectionGraph::k_blockSize; ++i) {
        outputs[0].values[i] = inputs[0].values[i] * inputs[1].values[i];
    }
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

void MulTri::block_process(uint32_t fs) {
    for (int i = 0; i < ConnectionGraph::k_blockSize; ++i) {
        outputs[0].values[i] = inputs[0].values[i] * inputs[1].values[i] * inputs[2].values[i];
    }
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

void MulQuad::block_process(uint32_t fs) {
    for (int i = 0; i < ConnectionGraph::k_blockSize; ++i) {
        outputs[0].values[i] = inputs[0].values[i] * inputs[1].values[i] * inputs[2].values[i] * inputs[3].values[i];
    }
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

void Gain::block_process(uint32_t fs) {
    for (int i = 0; i < ConnectionGraph::k_blockSize; ++i) {
        outputs[0].values[i] = inputs[0].values[i] * inputs[2].values[i];
        outputs[1].values[i] = inputs[1].values[i] * inputs[2].values[i];
    }
}
