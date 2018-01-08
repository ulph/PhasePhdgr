#include "crossfade.hpp"
#include "inlines.hpp"

CrossFade::CrossFade() {
    inputs.push_back(Pad("first"));
    inputs.push_back(Pad("second"));
    inputs.push_back(Pad("crossfade", 0.5));
    outputs.push_back(Pad("output"));
}
void CrossFade::process() {
    float mix = limit(inputs[2].value);
    outputs[0].value = (1.0f - mix)*inputs[0].value + mix*inputs[1].value;
}

FadeCross::FadeCross() {
    inputs.push_back(Pad("input"));
    inputs.push_back(Pad("fadecross", 0.5));
    outputs.push_back(Pad("first"));
    outputs.push_back(Pad("second"));
}
void FadeCross::process() {
    float mix = limit(inputs[1].value);
    outputs[0].value = inputs[0].value*(1.0f - mix);
    outputs[1].value = inputs[0].value*mix;
}
