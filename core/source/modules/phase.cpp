#include "phase.hpp"

Phase::Phase() : trig(0.f), phase(0.f){
    inputs.push_back(Pad("freq", "hz"));
    inputs.push_back(Pad("reset"));
    inputs.push_back(Pad("start", -1.f));
    outputs.push_back(Pad("phase"));
}

static inline float incrementPhase(float p, float p0, float f, float t, float t_old, float fs) {
    if (t > 0.f && t_old <= 0.f)
        p = p0;
    else if (fs) {
        p += 2 * f / (float)fs;
    }
    while (p > 1) { p -= 2; }
    while (p < -1) { p += 2; }
    return p;
}

void Phase::processSample(int sample)
{
    auto f_fs = (float)fs;

    float f = inputs[0].values[sample];
    float t = inputs[1].values[sample];
    float p0 = inputs[2].values[sample];

    phase = incrementPhase(phase, p0, f, t, trig, f_fs);
    trig = t;

    outputs[0].values[sample] = phase;
}

void Phase::processBlock()
{
    auto f_fs = (float)fs;
    
    for (int i = 0; i < Pad::k_blockSize; ++i) {
        float f = inputs[0].values[i];
        float t = inputs[1].values[i];
        float p0 = inputs[2].values[i];

        phase = incrementPhase(phase, p0, f, t, trig, f_fs);
        trig = t;

        outputs[0].values[i] = phase;
    }

}
