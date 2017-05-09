#include "effectchain.hpp"
#include "busmodules.hpp"
#include "moduleregister.hpp"
#include "design.hpp"
#include "parameters.hpp"

namespace PhasePhckr {

const vector<PadDescription> effectChainInBus = {
    {"left", "", 0},
    {"right", "", 0},
    {"mod", "", 0},
    {"exp", "", 0},
    {"brt", "", 0}
};

const vector<PadDescription> effectChainOutBus = {
    {"left", "", 0},
    {"right", "", 0},
};

EffectChain::EffectChain(const PatchDescriptor& fxChain, const ComponentRegister & cp) {
    PatchDescriptor patchDescriptor = fxChain;
    ModuleRegister::registerAllModules(connectionGraph);
    std::map<std::string, int> moduleHandles;
    designPatch(
        connectionGraph,
        patchDescriptor,
        effectChainInBus,
        effectChainOutBus,
        moduleHandles,
        cp
    );
    inBus = moduleHandles["inBus"];
    outBus = moduleHandles["outBus"];
}

void EffectChain::update(float * bufferL, float * bufferR, int numSamples, float sampleRate, const GlobalData& globalData) {
    GlobalData globalDataCopy = globalData;
    for (int i = 0; i < numSamples; ++i) {
        globalDataCopy.update();
        const GlobalDataState& g = globalDataCopy.getState();
        connectionGraph.setInput(inBus, 0, bufferL[i]);
        connectionGraph.setInput(inBus, 1, bufferR[i]);
        connectionGraph.setInput(inBus, 2, g.mod);
        connectionGraph.setInput(inBus, 3, g.exp);
        connectionGraph.setInput(inBus, 4, g.brt);

        connectionGraph.process(outBus, sampleRate);

        float sampleL = connectionGraph.getOutput(outBus, 0);
        float sampleR = connectionGraph.getOutput(outBus, 1);
        bufferL[i] = sampleL;
        bufferR[i] = sampleR;
    }
}

}
