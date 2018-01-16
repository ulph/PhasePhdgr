#include "zdf.hpp"
#include "inlines.hpp"

ZdfLp::ZdfLp() {
    inputs.push_back(Pad("x"));
    inputs.push_back(Pad("wc", 16000.f, "hz"));
    outputs.push_back(Pad("y"));
    outputs.push_back(Pad("v"));
}

void ZdfLp::process() {
    float x = inputs[0].value;
    float fc = limit(inputs[1].value, 1.0f, fs*0.5f);
    float wc = fc*fsInv*2.0f*M_PI;
    float g = prewarp(wc) * 0.5f;
    float v_ = (x - z1) / (1.0f + g);
    float v = v_*g;
    float y = v + z1;
    z1 = y + v;
    outputs[0].value = y;
    outputs[1].value = v_;
}