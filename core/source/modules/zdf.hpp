#pragma once

#include "module.hpp"
#include <math.h>

// Bilinear transposed direct form II, aka TPT, transformed digital filters

inline float prewarp(float wc, float fs, float fsInv) {
    return 2.0f * fs * tanf(wc * fsInv * 0.5f);
}

class ZdfLp : public ModuleCRTP<ZdfLp> {
    float z1 = 0.0f;
public:
    ZdfLp();
    virtual void process() override;
    static Module* factory() { return new ZdfLp(); }
};
