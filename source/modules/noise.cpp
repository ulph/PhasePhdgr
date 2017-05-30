#include "noise.hpp"

Noise::Noise() {
    inputs.push_back(Pad("bias", 2.f));
    inputs.push_back(Pad("scale", 1.f));
    outputs.push_back(Pad("out"));
    val = 01135; // seed
}

void Noise::process(uint32_t fs) {
    float rand = ((float)((val & 0x7fffffff) - 0x40000000)) * (float)(1.0 / 0x40000000);
    outputs[0].value = inputs[1].value*rand - 2.f + inputs[0].value;
    val = val * 435898247 + 382842987;
}
