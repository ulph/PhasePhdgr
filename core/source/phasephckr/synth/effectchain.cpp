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
        connectionGraph.setInput(p.first, 0, p.second.val);
    }

    GlobalData globalDataCopy = globalData;
    for (int j = 0; j < numSamples; ++j) {
        globalDataCopy.update();

        const GlobalDataState& g = globalDataCopy.getState();
        const GlobalTimeDataState &t = globalData.getTimeState();

        int i=0;
        connectionGraph.setInput(inBus, i++, bufferL[j]);
        connectionGraph.setInput(inBus, i++, bufferR[j]);

        connectionGraph.setInput(inBus, i++, g.mod);
        connectionGraph.setInput(inBus, i++, g.exp);
        connectionGraph.setInput(inBus, i++, g.brt);

        connectionGraph.setInput(inBus, i++, (float)t.nominator);
        connectionGraph.setInput(inBus, i++, (float)t.denominator);
        connectionGraph.setInput(inBus, i++, t.barLength);
        connectionGraph.setInput(inBus, i++, t.bpm);
        connectionGraph.setInput(inBus, i++, t.barPosition);
        connectionGraph.setInput(inBus, i++, t.position);
        connectionGraph.setInput(inBus, i++, t.time);

        connectionGraph.process(outBus, sampleRate);

        float sampleL = connectionGraph.getOutput(outBus, 0);
        float sampleR = connectionGraph.getOutput(outBus, 1);
        bufferL[j] = sampleL;
        bufferR[j] = sampleR;
    }
}

void EffectChain::setParameter(int handle, float value){
    auto it = parameterHandles.find(handle);
    if(it == parameterHandles.end()){
        assert(0);
        return;
    }
    it->second.val = value;
}

const ParameterHandleMap& EffectChain::getParameterHandles(){
    return parameterHandles;
}


}
