#include "crossfade.hpp"
#include "inlines.hpp"

CrossFade::CrossFade() {
    inputs.push_back(Pad("a"));
    inputs.push_back(Pad("b"));
    inputs.push_back(Pad("mix", 0.5));
    outputs.push_back(Pad("out"));
}
void CrossFade::processSample(int sample) {
    float mix = limit(inputs[2].values[sample]);
    outputs[0].values[sample] = (1.0f - mix)*inputs[0].values[sample] + mix*inputs[1].values[sample];
}

FadeCross::FadeCross() {
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("mix", 0.5));
    outputs.push_back(Pad("a"));
    outputs.push_back(Pad("b"));
}
void FadeCross::processSample(int sample) {
    float mix = limit(inputs[1].values[sample]);
    outputs[0].values[sample] = inputs[0].values[sample]*(1.0f - mix);
    outputs[1].values[sample] = inputs[0].values[sample]*mix;
}

static inline void muxProcess(float in, float select, std::vector<Pad>& outputs, int sample) {
    for (int i = 0; i < outputs.size(); i++) outputs[i].values[sample] = 0.0f;
    int idx = (int)(select * outputs.size());
    idx = idx >= outputs.size() ? (int)(outputs.size() - 1) : idx;
    idx = idx < 0 ? 0 : idx;
    outputs[idx].values[sample] = in;
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

void Mux8::processSample(int sample) {
    float in = inputs[0].values[sample];
    float select = limit(inputs[1].values[sample]);
    muxProcess(in, select, outputs, sample);
}

Mux4::Mux4() {
    inputs.push_back(Pad("in"));
    inputs.push_back(Pad("select"));
    outputs.push_back(Pad("a"));
    outputs.push_back(Pad("b"));
    outputs.push_back(Pad("c"));
    outputs.push_back(Pad("d"));
}

void Mux4::processSample(int sample) {
    float in = inputs[0].values[sample];
    float select = limit(inputs[1].values[sample]);
    muxProcess(in, select, outputs, sample);
}
