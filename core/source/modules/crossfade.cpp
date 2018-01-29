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

static inline void muxProcess(float in, float select, std::vector<Pad>& outputs) {
    for (int i = 0; i < outputs.size(); i++) outputs[i].value = 0.0f;
    int idx = (int)(select * outputs.size());
    idx = idx >= outputs.size() ? outputs.size() - 1 : idx;
    idx = idx < 0 ? 0 : idx;
    outputs[idx].value = in;
}

Mux8::Mux8() {
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("select"));
    outputs.push_back(Pad("a"));
    outputs.push_back(Pad("b"));
    outputs.push_back(Pad("c"));
    outputs.push_back(Pad("d"));
    outputs.push_back(Pad("e"));
    outputs.push_back(Pad("f"));
    outputs.push_back(Pad("g"));
    outputs.push_back(Pad("h"));
}

void Mux8::process() {
    float in = inputs[0].value;
    float select = limit(inputs[1].value);
    muxProcess(in, select, outputs);
}

Mux4::Mux4() {
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("select"));
    outputs.push_back(Pad("a"));
    outputs.push_back(Pad("b"));
    outputs.push_back(Pad("c"));
    outputs.push_back(Pad("d"));
}

void Mux4::process() {
    float in = inputs[0].value;
    float select = limit(inputs[1].value);
    muxProcess(in, select, outputs);
}
