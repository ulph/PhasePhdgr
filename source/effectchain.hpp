#pragma once

#include "connectiongraph.hpp"
#include "voicebus.hpp"
#include "design.hpp"

using namespace std;

namespace PhasePhckr {

class EffectChain {
private:
    ConnectionGraph connectionGraph;
    int inBus;
    int outBus;
    map<string, int> moduleHandles;
    map<string, int> parameterHandles;
public:
    EffectChain(const PatchDescriptor& fxChain, const ComponentRegister & cp);
    virtual void update(float * leftChannelbuffer, float * rightChannelbuffer, int numSamples, float sampleRate, const GlobalData& globalData);
    virtual ~EffectChain(){}

};

}
