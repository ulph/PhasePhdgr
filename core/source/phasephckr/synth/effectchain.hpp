#pragma once

#include "phasephckr/components.hpp"

#include "connectiongraph.hpp"
#include "voicebus.hpp"

#include "pluginsregister.hpp"

using namespace std;

namespace PhasePhckr {

class EffectChain {
private:
    ConnectionGraph connectionGraph;
    int inBus;
    int outBus;
    map<string, int> moduleHandles;
    ParameterHandleMap parameterHandles;
public:
    EffectChain(const PatchDescriptor& fxChain, const ComponentRegister & cp, const PluginsRegister * sdkReg);
    void setParameter(int handle, float value);
    const ParameterHandleMap& getParameterHandles();
    void update(float * leftChannelbuffer, float * rightChannelbuffer, int numSamples, float sampleRate, GlobalData& globalData);
    virtual ~EffectChain(){}
    void reset();
};

}
