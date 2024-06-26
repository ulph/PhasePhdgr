#include <stdlib.h>

#include "noise.hpp"

Noise::Noise() {
    inputs.push_back(Pad("bias", 0.f));
    inputs.push_back(Pad("scale", 1.f));
    outputs.push_back(Pad("out"));
    val = rand() % UINT32_MAX;
}

void Noise::process() {
    float rand = ((float)((val & 0x7fffffff) - 0x40000000)) * (float)(1.0 / 0x40000000);
    float value = rand*0.5f - 1.f;
    outputs[0].value = value*inputs[1].value + inputs[0].value;;
    val = val * 435898247 + 382842987;
}
