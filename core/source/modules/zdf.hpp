#pragma once

#include "module.hpp"

#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>

// Bilinear transposed direct form II, aka TPT, transformed digital filters

inline float prewarp(float wc) {
    // nyquist is at M_PI;
    assert(wc >= 0.0f);
    assert(wc <= M_PI);
    return 2.0f * tanf(wc * 0.5f);
}

class ZdfLp : public ModuleCRTP<ZdfLp> {
    float z1 = 0.0f;
public:
    ZdfLp();
    virtual void process() override;
    static Module* factory() { return new ZdfLp(); }
};
