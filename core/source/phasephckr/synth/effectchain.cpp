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
    const GlobalDataState& g = globalDataCopy.getState();
    const GlobalTimeDataState &t = globalData.getTimeState();

    connectionGraph.setInput(inBus, 5, (float)t.nominator);
    connectionGraph.setInput(inBus, 6, (float)t.denominator);
    connectionGraph.setInput(inBus, 7, t.barLength);
    connectionGraph.setInput(inBus, 8, t.bpm);
    connectionGraph.setInput(inBus, 9, t.barPosition);
    connectionGraph.setInput(inBus, 10, t.position);
    connectionGraph.setInput(inBus, 11, t.time);

    vector<ConnectionGraph::SampleBuffer> outBuffers;
    outBuffers.push_back({ outBus, 0, { 0.f } });
    outBuffers.push_back({ outBus, 1, { 0.f } });

    for (int j = 0; j < numSamples; j += ConnectionGraph::k_blockSize) {
        globalDataCopy.update();

        connectionGraph.block_setInput(inBus, 0, &bufferL[j]);
        connectionGraph.block_setInput(inBus, 1, &bufferR[j]);

        connectionGraph.setInput(inBus, 2, g.mod);
        connectionGraph.setInput(inBus, 3, g.exp);
        connectionGraph.setInput(inBus, 4, g.brt);

        connectionGraph.processBlock(outBus, sampleRate, outBuffers);
        for (int i = 0; i < ConnectionGraph::k_blockSize; ++i) {
            float sampleL = outBuffers[0].buf[i];
            float sampleR = outBuffers[1].buf[i];
            bufferL[j + i] = sampleL;
            bufferR[j + i] = sampleR;
        }
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
