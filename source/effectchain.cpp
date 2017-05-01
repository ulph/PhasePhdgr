#include "effectchain.hpp"
#include "busmodules.hpp"
#include "moduleregister.hpp"
#include "design.hpp"
#include "parameters.hpp"

namespace PhasePhckr {

EffectChain::EffectChain(const ConnectionGraphDescriptor& fxChain, const ComponentRegister & cp) {
    ConnectionGraphDescriptor graphDescriptor = fxChain;

    connectionGraph.registerModule(c_EffectInput.type, &EffectInputBus::factory);
    connectionGraph.registerModule(c_EffectOutput.type, &StereoOutBus::factory);
    ModuleRegister::registerAllModules(connectionGraph);

    graphDescriptor.modules.emplace_back(c_EffectInput);
    graphDescriptor.modules.emplace_back(c_EffectOutput);

    std::map<std::string, int> moduleHandles;
    designConnectionGraph(
        connectionGraph,
        graphDescriptor,
        moduleHandles,
        cp
    );

    inBus = moduleHandles[c_EffectInput.name];
    outBus = moduleHandles[c_EffectOutput.name];
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
