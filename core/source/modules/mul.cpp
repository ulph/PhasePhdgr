#include "mul.hpp"

Mul::Mul()
{
    inputs.push_back(Pad("in", 1.f));
    inputs.push_back(Pad("in2", 1.f));
    outputs.push_back(Pad("out"));
}

void Mul::processSample(int sample) {
    outputs[0].values[sample] = inputs[0].values[sample] * inputs[1].values[sample];
}

void Mul::processBlock() {
    for (int i = 0; i < Pad::k_blockSize; ++i) {
        outputs[0].values[i] = inputs[0].values[i] * inputs[1].values[i];
    }
}

MulTri::MulTri()
{
    inputs.push_back(Pad("in", 1.f));
    inputs.push_back(Pad("in2", 1.f));
    inputs.push_back(Pad("in3", 1.f));
    outputs.push_back(Pad("out"));
}

void MulTri::processSample(int sample) {
    outputs[0].values[sample] = inputs[0].values[sample] * inputs[1].values[sample] * inputs[2].values[sample];
}

void MulTri::processBlock() {
    for (int i = 0; i < Pad::k_blockSize; ++i) {
        outputs[0].values[i] = inputs[0].values[i] * inputs[1].values[i] * inputs[2].values[i];
    }
}

MulQuad::MulQuad()
{
    inputs.push_back(Pad("in", 1.f));
    inputs.push_back(Pad("in2", 1.f));
    inputs.push_back(Pad("in3", 1.f));
    inputs.push_back(Pad("in4", 1.f));
    outputs.push_back(Pad("out"));
}

void MulQuad::processSample(int sample) {
    outputs[0].values[sample] = inputs[0].values[sample] * inputs[1].values[sample] * inputs[2].values[sample] * inputs[3].values[sample];
}

void MulQuad::processBlock() {
    for (int i = 0; i < Pad::k_blockSize; ++i) {
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

void Gain::processSample(int sample) {
    outputs[0].values[sample] = inputs[0].values[sample] * inputs[2].values[sample];
    outputs[1].values[sample] = inputs[1].values[sample] * inputs[2].values[sample];
}

void Gain::processBlock() {
    for (int i = 0; i < Pad::k_blockSize; ++i) {
        outputs[0].values[i] = inputs[0].values[i] * inputs[2].values[i];
        outputs[1].values[i] = inputs[1].values[i] * inputs[2].values[i];
    }
}
