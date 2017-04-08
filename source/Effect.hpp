#pragma once

namespace PhasePhckr {

class EffectI {
public:
    virtual void reset() = 0;
    virtual void update(float * buffer, int numSamples, float sampleRate) = 0;
};

}