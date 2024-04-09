#include "div.hpp"
#include <math.h>

Div::Div()
{
    inputs.push_back(Pad("nom", 1.f));
    inputs.push_back(Pad("den", 1.f));
    outputs.push_back(Pad("div"));
    outputs.push_back(Pad("NaN"));
}

void Div::processSample(int sample) {
    auto n = inputs[0].values[sample];
    auto d = inputs[1].values[sample];
    if (d == 0.f) {
        outputs[0].values[sample] = 0.f;
        outputs[1].values[sample] = 1.f;
        return;
    }
    outputs[0].values[sample] = n / d;
    outputs[1].values[sample] = 0.f;
}


Mod::Mod()
{
    inputs.push_back(Pad("nom", 1.f));
    inputs.push_back(Pad("den", 1.f));
    outputs.push_back(Pad("rem"));
    outputs.push_back(Pad("NaN"));
}

void Mod::processSample(int sample) {
    auto n = inputs[0].values[sample];
    auto d = inputs[1].values[sample];
    if (d == 0.f) {
        outputs[0].values[sample] = 0.f;
        outputs[1].values[sample] = 1.f;
        return;
    }
    outputs[0].values[sample] = fmod(n, d);
    outputs[1].values[sample] = 0.f;
}

// TODO, floor, ceil and round (which all also return the remainder)
