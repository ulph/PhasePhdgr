#pragma once

#include "connectiongraph.hpp"
#include "voicebus.hpp"
#include "design.hpp"

namespace PhasePhckr {

class EffectChain {
private:
    ConnectionGraph connectionGraph;
    int inBus;
    int outBus;

public:
    EffectChain(const ConnectionGraphDescriptor& fxChain);
    virtual void update(float * leftChannelbuffer, float * rightChannelbuffer, int numSamples, float sampleRate, const GlobalData& globalData);

};

}