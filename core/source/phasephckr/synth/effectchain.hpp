#pragma once

#include "phasephckr/components.hpp"

#include "connectiongraph.hpp"
#include "voicebus.hpp"

using namespace std;

namespace PhasePhckr {

class EffectChain {
private:
    ConnectionGraph connectionGraph;
    int inBus;
    int outBus;
    map<string, int> moduleHandles;
    map<string, int> parameterHandles;
    map<int, float> parameterValues;
public:
    EffectChain(const PatchDescriptor& fxChain, const ComponentRegister & cp);
    void setParameter(int handle, float value);
    const map<string, int>& getParameterHandles();
    void update(float * leftChannelbuffer, float * rightChannelbuffer, int numSamples, float sampleRate, const GlobalData& globalData);
    virtual ~EffectChain(){}
};

}
