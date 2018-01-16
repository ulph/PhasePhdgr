#include "crossfade.hpp"
#include "inlines.hpp"

CrossFade::CrossFade() {
    inputs.push_back(Pad("a"));
    inputs.push_back(Pad("b"));
    inputs.push_back(Pad("mix", 0.5));
    outputs.push_back(Pad("out"));
}
void CrossFade::process() {
    float mix = limit(inputs[2].value);
    outputs[0].value = (1.0f - mix)*inputs[0].value + mix*inputs[1].value;
}

FadeCross::FadeCross() {
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("mix", 0.5));
    outputs.push_back(Pad("a"));
    outputs.push_back(Pad("b"));
}
void FadeCross::process() {
    float mix = limit(inputs[1].value);
    outputs[0].value = inputs[0].value*(1.0f - mix);
    outputs[1].value = inputs[0].value*mix;
}
