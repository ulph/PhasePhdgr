#include "noise.hpp"

Noise::Noise() {
    outputs.push_back(Pad("out"));
    val = 01135; // seed
}

void Noise::process(uint32_t fs) {
    outputs[0].value = ((float)((val & 0x7fffffff) - 0x40000000)) *
                      (float)(1.0 / 0x40000000);
    val = val * 435898247 + 382842987;
}
