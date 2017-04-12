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
                ModuleVariable{"outPreGain", "GAIN"},
                ModuleVariable{"outSaturation", "SSATAN"},
                ModuleVariable{"outPostGain", "GAIN"},
                ModuleVariable{"leftDelay", "DELAY" },
                ModuleVariable{"rightDelay", "DELAY" },
            },
            std::vector<ModulePortConnection>{
                ModulePortConnection{{"inBus", "left"}, {"outPreGain", "left"}},
                ModulePortConnection{{"inBus", "right"}, {"outPreGain", "right"}},

                ModulePortConnection{{"inBus", "left"}, {"leftDelay", "in"}},
                ModulePortConnection{{"inBus", "right"}, {"rightDelay", "in"}},
                ModulePortConnection{{"leftDelay", "out"}, {"rightDelay", "in"}},
                ModulePortConnection{{"rightDelay", "out"}, {"leftDelay", "in"}},

                ModulePortConnection{{"leftDelay", "out"}, {"outPreGain", "left"}},
                ModulePortConnection{{"rightDelay", "out"}, {"outPreGain", "right"}},

                ModulePortConnection{{"outPreGain", "left"}, {"outSaturation", "left"}},
                ModulePortConnection{{"outPreGain", "right"}, {"outSaturation", "right"}},
                ModulePortConnection{{"outSaturation", "left"}, {"outPostGain", "left"}},
                ModulePortConnection{{"outSaturation", "right"}, {"outPostGain", "right"}},
                ModulePortConnection{{"outPostGain", "left"}, {"outBus", "left"}},
                ModulePortConnection{{"outPostGain", "right"}, {"outBus", "right"}}
            },
            std::vector<ModulePortValue>{
                ModulePortValue{"outPreGain", "gain", 0.5f},
                ModulePortValue{"outPostGain", "gain", 0.5f},
                ModulePortValue{"leftDelay", "time", 0.22f},
                ModulePortValue{"rightDelay", "time", 0.45f},
                ModulePortValue{"leftDelay", "gain", 0.75f},
                ModulePortValue{"rightDelay", "gain", 0.75f},
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
