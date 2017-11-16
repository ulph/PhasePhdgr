#include "div.hpp"

Div::Div()
{
    inputs.push_back(Pad("nom", 1.f));
    inputs.push_back(Pad("den", 1.f));
    outputs.push_back(Pad("div"));
    outputs.push_back(Pad("NaN"));
}

void Div::process(uint32_t fs) {
    auto n = inputs[0].value;
    auto d = inputs[1].value;
    if (d == 0.f) {
        outputs[0].value = 0.f;
        outputs[1].value = 1.f;
        return;
    }
    outputs[0].value = n / d;
    outputs[1].value = 0.f;
}


Mod::Mod()
{
    inputs.push_back(Pad("nom", 1.f));
    inputs.push_back(Pad("den", 1.f));
    outputs.push_back(Pad("rem"));
    outputs.push_back(Pad("NaN"));
}

void Mod::process(uint32_t fs) {
    auto n = inputs[0].value;
    auto d = inputs[1].value;
    if (d == 0.f) {
        outputs[0].value = 0.f;
        outputs[1].value = 1.f;
        return;
    }
    outputs[0].value = fmod(n, d);
    outputs[1].value = 0.f;
}

// TODO, floor, ceil and round (which all also return the remainder)