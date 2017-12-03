#include "phase.hpp"

Phase::Phase() : trig(0.f), phase(0.f){
    inputs.push_back(Pad("freq"));
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

void Phase::process(uint32_t fs)
{
    auto f_fs = (float)fs;

    float f = inputs[0].value;
    float t = inputs[1].value;
    float p0 = inputs[2].value;

    phase = incrementPhase(phase, p0, f, t, trig, f_fs);
    trig = t;

    outputs[0].value = phase;
}

void Phase::block_process(uint32_t fs)
{
    auto f_fs = (float)fs;
    
    for (int i = 0; i < ConnectionGraph::k_blockSize; ++i) {
        float f = inputs[0].values[i];
        float t = inputs[1].values[i];
        float p0 = inputs[2].values[i];

        phase = incrementPhase(phase, p0, f, t, trig, f_fs);
        trig = t;

        outputs[0].values[i] = phase;
    }

}