#include "abs.hpp"

Abs::Abs()
{
    inputs.push_back(Pad("input"));
    outputs.push_back(Pad("abs"));
}

void Abs::process(uint32_t fs) {
    outputs[0].value = fabs(inputs[0].value);
}
