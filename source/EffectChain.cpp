#include "EffectChain.hpp"
#include "BusModules.hpp"
#include "moduleregister.hpp"
#include "DesignConnectionGraph.hpp"
#include "MPEVoice.hpp"

namespace PhasePhckr {

EffectChain::EffectChain(){
    connectionGraph.registerModule("STEREOBUS", &StereoBus::factory);
    connectionGraph.registerModule("EFFECTINPUTBUS", &EffectInputBus::factory);
    ModuleRegister::registerAllModules(connectionGraph);

    BusHandles bus = DesignConnectionGraph(
        connectionGraph,
        Patch{
            ModuleVariable{"inBus", "EFFECTINPUTBUS"},
            ModuleVariable{"outBus", "STEREOBUS"},
            std::vector<ModuleVariable>{
                ModuleVariable{"outGain", "GAIN"},
                ModuleVariable{"outSaturation", "SSATAN"}
            },
            std::vector<ModulePortConnection>{
                ModulePortConnection{{"inBus", "left"}, {"outSaturation", "left"}},
                ModulePortConnection{{"inBus", "right"}, {"outSaturation", "right"}},
                ModulePortConnection{{"outSaturation", "left"}, {"outGain", "left"}},
                ModulePortConnection{{"outSaturation", "right"}, {"outGain", "right"}},
                ModulePortConnection{{"outGain", "left"}, {"outBus", "left"}},
                ModulePortConnection{{"outGain", "right"}, {"outBus", "right"}}
            },
            std::vector<ModulePortValue>{
                ModulePortValue{"outGain", "gain", 0.5},
            }
        }
    );

    inBus = bus.inBus;
    outBus = bus.outBus;
}

void EffectChain::update(float * bufferL, float * bufferR, int numSamples, float sampleRate, const GlobalData& globalData){
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
