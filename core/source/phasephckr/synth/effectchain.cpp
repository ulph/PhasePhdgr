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
        patchDescriptor.componentBundle,
        c_effectChainInBus,
        c_effectChainOutBus,
        moduleHandles,
        SynthGraphType::EFFECT,
        parameterHandles,
        cp
    );
    
    inBus = moduleHandles[c_inBus.name];
    outBus = moduleHandles[c_outBus.name];
}

void EffectChain::update(float * bufferL, float * bufferR, int numSamples, float sampleRate, GlobalData& globalData) {
    assert(numSamples % ConnectionGraph::k_blockSize == 0);

    for(const auto& p: parameterHandles){
        connectionGraph.setInput(p.first, 0, p.second.v.val);
    }

    const GlobalDataState& g = globalData.getState();
    const GlobalTimeDataState &t = globalData.getTimeState();

    connectionGraph.setInput(inBus, 5, (float)t.nominator);
    connectionGraph.setInput(inBus, 6, (float)t.denominator);
    connectionGraph.setInput(inBus, 7, t.barLength);
    connectionGraph.setInput(inBus, 8, t.bpm);
    connectionGraph.setInput(inBus, 9, t.barPosition);
    connectionGraph.setInput(inBus, 10, t.position);
    connectionGraph.setInput(inBus, 11, t.time);

    int j = 0;
    for (j = 0; (j + ConnectionGraph::k_blockSize) <= numSamples; j += ConnectionGraph::k_blockSize) {
        globalData.update();

        connectionGraph.setInputBlock(inBus, 2, g.mod);
        connectionGraph.setInputBlock(inBus, 3, g.exp);
        connectionGraph.setInputBlock(inBus, 4, g.brt);

        connectionGraph.setInputBlock(inBus, 0, &bufferL[j]);
        connectionGraph.setInputBlock(inBus, 1, &bufferR[j]);

        connectionGraph.processBlock(outBus, sampleRate);

        connectionGraph.getOutputBlock(outBus, 0, &bufferL[j]);
        connectionGraph.getOutputBlock(outBus, 1, &bufferR[j]);
    }

    assert(j == numSamples);
}

void EffectChain::setParameter(int handle, float value){
    auto it = parameterHandles.find(handle);
    if(it == parameterHandles.end()){
        assert(0);
        return;
    }
    it->second.v.val = value;
}

const ParameterHandleMap& EffectChain::getParameterHandles(){
    return parameterHandles;
}

void EffectChain::reset() {
    connectionGraph.reset();
}

}
