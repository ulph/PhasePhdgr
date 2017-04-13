#include "EffectChain.hpp"
#include "BusModules.hpp"
#include "moduleregister.hpp"
#include "DesignConnectionGraph.hpp"
#include "parameters.hpp"

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
                ModuleVariable{"delayGain", "GAIN"},
                ModuleVariable{"leftDelay", "DELAY" },
                ModuleVariable{"rightDelay", "DELAY" },
                ModuleVariable{"lfoPhase", "PHASE" },
                ModuleVariable{"lfo", "SINE" },
                ModuleVariable{"delayLeftTime", "SCLSHFT"},
                ModuleVariable{"delayRightTime", "SCLSHFT"}, 
                ModuleVariable{"leftDelayLP", "RCLP"},
                ModuleVariable{"rightDelayLP", "RCLP"},
                ModuleVariable{"leftDelayHP", "RCHP" },
                ModuleVariable{"rightDelayHP", "RCHP" },
            },
            std::vector<ModulePortConnection>{
                ModulePortConnection{{"inBus", "left"}, {"outPreGain", "left"}},
                ModulePortConnection{{"inBus", "right"}, {"outPreGain", "right"}},

                // a cross feedback stereo delay
                ModulePortConnection{{"outPreGain", "left"}, {"leftDelayHP", "in"}},
                ModulePortConnection{{"outPreGain", "right"}, {"rightDelayHP", "in"}},
                ModulePortConnection{{"leftDelay", "out"}, {"leftDelayHP", "in"}},
                ModulePortConnection{{"rightDelay", "out"}, {"rightDelayHP", "in"}},
                ModulePortConnection{{"leftDelayHP", "out"}, {"leftDelayLP", "in"}},
                ModulePortConnection{{"rightDelayHP", "out"}, {"rightDelayLP", "in"}},
                ModulePortConnection{{"leftDelayLP", "out"}, {"rightDelay", "in"}},
                ModulePortConnection{{"rightDelayLP", "out"}, {"leftDelay", "in"}},
                // - output and "wet" factor
                ModulePortConnection{{"leftDelayLP", "out"}, {"delayGain", "left"}},
                ModulePortConnection{{"rightDelayLP", "out"}, {"delayGain", "right"}},
                ModulePortConnection{{"delayGain", "left"}, {"outPreGain", "left"}},
                ModulePortConnection{{"delayGain", "right"}, {"outPreGain", "right"}},
                // - time modulation, causes artifact due to no resampling ...
                ModulePortConnection{{"lfoPhase", "phase"}, {"lfo", "phase"}},
                ModulePortConnection{{"lfo", "sine"}, {"delayLeftTime", "input"}},
                ModulePortConnection{{"lfo", "sine"}, {"delayRightTime", "input"}},
                ModulePortConnection{{"delayLeftTime", "output"}, {"leftDelay", "time"}},
                ModulePortConnection{{"delayRightTime", "output"}, {"rightDelay", "time"}},

                // saturating gain stage
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

                // delay time moduluation
                ModulePortValue{"lfoPhase", "freq", 2.0f},
                ModulePortValue{"delayLeftTime", "shift", 0.22f},
                ModulePortValue{"delayRightTime", "shift", 0.45f},
                ModulePortValue{"delayLeftTime", "scale", 0.001f},
                ModulePortValue{"delayRightTime", "scale", 0.001f},
                // - feedback amount
                ModulePortValue{"leftDelay", "gain", 0.56f},
                ModulePortValue{"rightDelay", "gain", 0.62f},
                // - lowpass of feedback loop (sounds good, but also AA ...)
                ModulePortValue{"leftDelayLP", "freq", 8500.0f},
                ModulePortValue{"rightDelayLP", "freq", 7500.0f},
                // - lowpass of feeback loop (sounds good, but also AA ...)
                ModulePortValue{"leftDelayHP", "freq", 350.0f},
                ModulePortValue{"rightDelayHP", "freq", 450.0f},
                // - effect "wet amount"
                ModulePortValue{"delayGain", "gain", 0.5f},
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
