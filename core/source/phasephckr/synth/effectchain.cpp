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
        EFFECT,
        parameterHandles,
        cp
    );
    inBus = moduleHandles[c_inBus.name];
    outBus = moduleHandles[c_outBus.name];
}

void EffectChain::update(float * bufferL, float * bufferR, int numSamples, float sampleRate, const GlobalData& globalData) {
    for(const auto& p: parameterHandles){
        connectionGraph.setInput(p.first, 0, p.second.value);
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
    auto it = parameterHandles.find(handle);
    if(it == parameterHandles.end()){
        assert(0);
        return;
    }
    it->second.value = value;
}

const ParameterHandleMap& EffectChain::getParameterHandles(){
    return parameterHandles;
}


}
