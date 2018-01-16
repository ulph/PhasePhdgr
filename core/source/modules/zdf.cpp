#include "zdf.hpp"
#include "inlines.hpp"

ZdfLp::ZdfLp() {
    inputs.push_back(Pad("x"));
    inputs.push_back(Pad("wc", 16000.f, "hz"));
    outputs.push_back(Pad("y"));
}

void ZdfLp::process() {
    float x = inputs[0].value;
    float wc = limitLow(inputs[1].value);
    float g = prewarp(wc, fs, fsInv) * fsInv * 0.5f;
    float v = (x - z1) * g / (1.0f + g);
    float y = v + z1;
    z1 = y + v;
    outputs[0].value = y;
}