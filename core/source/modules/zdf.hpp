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

class Zdf1p : public ModuleCRTP<Zdf1p> {
    float z1 = 0.0f;
public:
    Zdf1p();
    virtual void process() override;
    static Module* factory() { return new Zdf1p(); }
};
