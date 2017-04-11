#pragma once

#include "PhasePhckr.h"

namespace PhasePhckr {

class EffectChain {
public:
    virtual void reset();
    virtual void update(float * leftChannelbuffer, float * rightChannelbuffer, int numSamples, float sampleRate, const GlobalData& globalData);
};

}
