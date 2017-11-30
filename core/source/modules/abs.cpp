#include "abs.hpp"

Abs::Abs()
{
    inputs.push_back(Pad("input"));
    outputs.push_back(Pad("abs"));
}

void Abs::process(uint32_t fs) {
    outputs[0].value = fabsf(inputs[0].value);
}

void Abs::block_process(uint32_t fs) {
    for (int i = 0; i < ConnectionGraph::k_blockSize; ++i) {
        outputs[0].values[i] = fabsf(inputs[0].values[i]);
    }
}