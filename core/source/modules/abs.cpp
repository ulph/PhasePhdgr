#include "abs.hpp"

Abs::Abs()
{
    inputs.push_back(Pad("input"));
    outputs.push_back(Pad("abs"));
}

void Abs::process() {
    outputs[0].value = fabsf(inputs[0].value);
}

void Abs::block_process() {
    for (int i = 0; i < Pad::k_blockSize; ++i) {
        outputs[0].values[i] = fabsf(inputs[0].values[i]);
    }
}