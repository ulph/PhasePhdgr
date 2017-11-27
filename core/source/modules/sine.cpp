#include "sine.hpp"

Sine::Sine()
{
    inputs.push_back(Pad("phase"));
    outputs.push_back(Pad("sine"));
}

void Sine::process(uint32_t fs)
{
    outputs[0].value = sinf(float(M_PI) * inputs[0].value);
}

void Sine::block_process(uint32_t fs)
{
    for (size_t i = 0; i < ConnectionGraph::k_blockSize; ++i) {
        outputs[0].values[i] = sinf(float(M_PI) * inputs[0].values[i]);
    }
}
