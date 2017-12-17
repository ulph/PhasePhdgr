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

    inBuffers.push_back({ inBus, 0, nullptr });
    inBuffers.push_back({ inBus, 1, nullptr });

    outBuffers.push_back({ outBus, 0, nullptr });
    outBuffers.push_back({ outBus, 1, nullptr });
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
    for (j; (j + ConnectionGraph::k_blockSize) <= numSamples; j += ConnectionGraph::k_blockSize) {
        globalData.update();

        connectionGraph.setInputBlock(inBus, 2, g.mod);
        connectionGraph.setInputBlock(inBus, 3, g.exp);
        connectionGraph.setInputBlock(inBus, 4, g.brt);

        inBuffers[0].bufPtr = &bufferL[j];
        inBuffers[1].bufPtr = &bufferR[j];

        outBuffers[0].bufPtr = &bufferL[j]; 
        outBuffers[1].bufPtr = &bufferR[j];

        connectionGraph.processBlock(outBus, sampleRate, inBuffers, outBuffers);
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


}
