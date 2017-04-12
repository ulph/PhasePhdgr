#pragma once

#include "connectiongraph.hpp"
#include "VoiceBus.hpp"

namespace PhasePhckr {

class EffectChain {
private:
    ConnectionGraph connectionGraph;
    int inBus;
    int outBus;

public:
    EffectChain();
    virtual void update(float * leftChannelbuffer, float * rightChannelbuffer, int numSamples, float sampleRate, const GlobalData& globalData);

};

}
