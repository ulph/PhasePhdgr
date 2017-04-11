#include "EffectChain.hpp"
#include "BusModules.hpp"
#include "moduleregister.hpp"

namespace PhasePhckr {

EffectChain::EffectChain(){
    connectionGraph.registerModule("VOICEINPUT", &VoiceInputBus::factory);
    connectionGraph.registerModule("STEREOBUS", &StereoBus::factory);
    ModuleRegister::registerAllModules(connectionGraph);
    inBus = connectionGraph.addModule("STEREOBUS");
    outBus = connectionGraph.addModule("STEREOBUS");

    // stupid passthrough for now
    connectionGraph.connect(inBus, "Left", outBus, "Left");
    connectionGraph.connect(inBus, "Right", outBus, "Right");
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
