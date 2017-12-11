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

    inBuffers.push_back({ inBus, 0,{ 0.f } });
    inBuffers.push_back({ inBus, 1,{ 0.f } });

    outBuffers.push_back({ outBus, 0,{ 0.f } });
    outBuffers.push_back({ outBus, 1,{ 0.f } });
}

void EffectChain::update(float * bufferL, float * bufferR, int numSamples, float sampleRate, const GlobalData& globalData) {
    for(const auto& p: parameterHandles){
        connectionGraph.setInput(p.first, 0, p.second.v.val);
    }

    GlobalData globalDataCopy = globalData;
    const GlobalDataState& g = globalDataCopy.getState();
    const GlobalTimeDataState &t = globalData.getTimeState();

    connectionGraph.setInput(inBus, 5, (float)t.nominator);
    connectionGraph.setInput(inBus, 6, (float)t.denominator);
    connectionGraph.setInput(inBus, 7, t.barLength);
    connectionGraph.setInput(inBus, 8, t.bpm);
    connectionGraph.setInput(inBus, 9, t.barPosition);
    connectionGraph.setInput(inBus, 10, t.position);
    connectionGraph.setInput(inBus, 11, t.time);

    for (int j = 0; j < numSamples; j += ConnectionGraph::k_blockSize) {
        globalDataCopy.update();

        connectionGraph.setInput(inBus, 2, g.mod);
        connectionGraph.setInput(inBus, 3, g.exp);
        connectionGraph.setInput(inBus, 4, g.brt);

        memcpy(inBuffers[0].buf, &bufferL[j], sizeof(float)*ConnectionGraph::k_blockSize);
        memcpy(inBuffers[1].buf, &bufferR[j], sizeof(float)*ConnectionGraph::k_blockSize);

        connectionGraph.processBlock(outBus, sampleRate, inBuffers, outBuffers);

        memcpy(&bufferL[j], outBuffers[0].buf, sizeof(float)*ConnectionGraph::k_blockSize);
        memcpy(&bufferR[j], outBuffers[1].buf, sizeof(float)*ConnectionGraph::k_blockSize);

    }
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
