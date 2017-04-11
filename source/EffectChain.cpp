#include "EffectChain.hpp"
#include "BusModules.hpp"
#include "moduleregister.hpp"
#include "DesignConnectionGraph.hpp"

namespace PhasePhckr {

EffectChain::EffectChain(){
    connectionGraph.registerModule("STEREOBUS", &StereoBus::factory);
    ModuleRegister::registerAllModules(connectionGraph);

    BusHandles bus = DesignConnectionGraph(
        connectionGraph,
        ModuleVariable{"inBus", "STEREOBUS"},
        ModuleVariable{"outBus", "STEREOBUS"},
        std::vector<ModuleVariable>(),
        std::vector<ModulePortConnection>{
            ModulePortConnection{"inBus", "Left", "outBus", "Left"},
            ModulePortConnection{"inBus", "Right", "outBus", "Right"},
        },
        std::vector<ModulePortValue>()
    );

    inBus = bus.inBus;
    outBus = bus.outBus;
}

void EffectChain::update(float * bufferL, float * bufferR, int numSamples, float sampleRate, const GlobalData& globalData){
    for (int i = 0; i < numSamples; ++i) {
        connectionGraph.setInput(inBus, 0, bufferL[i]);
        connectionGraph.setInput(inBus, 1, bufferR[i]);
        connectionGraph.setInput(inBus, 2, globalData.mod);
        connectionGraph.setInput(inBus, 3, globalData.exp);
        connectionGraph.setInput(inBus, 4, globalData.brt);

        connectionGraph.process(outBus, sampleRate);

        float sampleL = connectionGraph.getOutput(outBus, 0);
        float sampleR = connectionGraph.getOutput(outBus, 1);
        bufferL[i] = sampleL;
        bufferR[i] = sampleR;
    }
}

}
