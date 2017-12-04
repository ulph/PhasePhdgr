#include "saturatoratan.hpp"

Atan::Atan()
{
    inputs.push_back(Pad("in"));
    outputs.push_back(Pad("out"));
}

void Atan::process(uint32_t fs)
{
    outputs[0].value = atanf(inputs[0].value);
}

void Atan::block_process(uint32_t fs)
{
    for (size_t i = 0; i < ConnectionGraph::k_blockSize; ++i) {
        outputs[0].values[i] = atanf(inputs[0].values[i]);
    }
}


SaturatorAtan::SaturatorAtan()
{
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("prescaler", 1.0f));
    outputs.push_back(Pad("out"));
}

void SaturatorAtan::process(uint32_t fs)
{
    float s = inputs[1].value;
    float f = 1.0f/atanf(s);
    outputs[0].value = atanf(inputs[0].value * s) * f;
}

void SaturatorAtan::block_process(uint32_t fs)
{
    for (size_t i = 0; i < ConnectionGraph::k_blockSize; ++i) {
        float s = inputs[1].values[i];
        float f = 1.0f / atanf(s);
        outputs[0].values[i] = atanf(inputs[0].values[i] * s) * f;
    }
}


StereoSaturatorAtan::StereoSaturatorAtan()
{
    inputs.push_back(Pad("left"));
    inputs.push_back(Pad("right"));
    inputs.push_back(Pad("prescaler", 1.0f));
    outputs.push_back(Pad("left"));
    outputs.push_back(Pad("right"));
}

void StereoSaturatorAtan::process(uint32_t fs)
{
    float s = inputs[2].value;
    float f = 1.0f/atanf(s);
    outputs[0].value = atanf(inputs[0].value * s) * f;
    outputs[1].value = atanf(inputs[1].value * s) * f;
}

void StereoSaturatorAtan::block_process(uint32_t fs)
{
    for (size_t i = 0; i < ConnectionGraph::k_blockSize; ++i) {
        float s = inputs[2].values[i];
        float f = 1.0f / atanf(s);
        outputs[0].values[i] = atanf(inputs[0].values[i] * s) * f;
        outputs[1].values[i] = atanf(inputs[1].values[i] * s) * f;
    }
}