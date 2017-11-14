#include "phasephckr/components.hpp"
#include "phasephckr/design.hpp"

#include "effectchain.hpp"
#include "busmodules.hpp"
#include "moduleregister.hpp"
#include "parameters.hpp"

namespace PhasePhckr {

EffectChain::EffectChain(const PatchDescriptor& fxChain, const ComponentRegister & cp) {
    PatchDescriptor patchDescriptor = fxChain;
    ModuleRegister::registerAllModules(connectionGraph);
    moduleHandles.clear();
    parameterHandles.clear();
    designPatch(
        connectionGraph,
        patchDescriptor,
        c_effectChainInBus,
        c_effectChainOutBus,
        moduleHandles,
        parameterHandles,
        cp
    );
    inBus = moduleHandles[c_inBus.name];
    outBus = moduleHandles[c_outBus.name];

    parameterValues.clear();
    for(const auto &p : parameterHandles){
        parameterValues[p.second] = 0.f;
    }
}

void EffectChain::update(float * bufferL, float * bufferR, int numSamples, float sampleRate, const GlobalData& globalData) {
    for(const auto& p: parameterValues){
        connectionGraph.setInput(p.first, 0, p.second);
    }

    GlobalData globalDataCopy = globalData;
    for (int i = 0; i < numSamples; ++i) {
        globalDataCopy.update();
        connectionGraph.setInput(inBus, 0, bufferL[i]);
        connectionGraph.setInput(inBus, 1, bufferR[i]);

        const GlobalDataState& g = globalDataCopy.getState();
        connectionGraph.setInput(inBus, 2, g.mod);
        connectionGraph.setInput(inBus, 3, g.exp);
        connectionGraph.setInput(inBus, 4, g.brt);

        const GlobalTimeDataState &t = globalData.getTimeState();
        connectionGraph.setInput(inBus, 5, (float)t.nominator);
        connectionGraph.setInput(inBus, 6, (float)t.denominator);
        connectionGraph.setInput(inBus, 7, t.bpm);
        connectionGraph.setInput(inBus, 8, t.position);
        connectionGraph.setInput(inBus, 9, t.time);

        connectionGraph.process(outBus, sampleRate);

        float sampleL = connectionGraph.getOutput(outBus, 0);
        float sampleR = connectionGraph.getOutput(outBus, 1);
        bufferL[i] = sampleL;
        bufferR[i] = sampleR;
    }
}

void EffectChain::setParameter(int handle, float value){
    auto it = parameterValues.find(handle);
    if(it == parameterValues.end()){
        assert(0);
        return;
    }
    it->second = value;
}

const map<string, int>& EffectChain::getParameterHandles(){
    return parameterHandles;
}


}
