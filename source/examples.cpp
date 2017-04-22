#include "design.hpp"

namespace PhasePhckr {

    const ComponentDescriptor stereoTapeDelay = {

        std::vector<ModulePortAlias> {
            ModulePortAlias{"left", std::vector<ModulePort>{{"leftDelay", "left"}}},
            ModulePortAlias{"right", std::vector<ModulePort>{{"rightDelay", "right"}}},
        },

        std::vector<ModulePortAlias>{
            ModulePortAlias{"left", std::vector<ModulePort>{{"delayGain", "left"}}},
            ModulePortAlias{"right", std::vector<ModulePort>{{"delayGain", "right"}}},
        },

        ConnectionGraphDescriptor{
            std::vector<ModuleVariable>{
                ModuleVariable{ "delayGain", "GAIN" },
                ModuleVariable{ "leftDelay", "DELAY" },
                ModuleVariable{ "rightDelay", "DELAY" },
                ModuleVariable{ "lfoPhase", "PHASE" },
                ModuleVariable{ "lfo", "SINE" },
                ModuleVariable{ "delayLeftTime", "SCLSHFT" },
                ModuleVariable{ "delayRightTime", "SCLSHFT" },
                ModuleVariable{ "leftDelayLP", "RCLP" },
                ModuleVariable{ "rightDelayLP", "RCLP" },
                ModuleVariable{ "leftDelayHP", "RCHP" },
                ModuleVariable{ "rightDelayHP", "RCHP" }
            },
            std::vector<ModulePortConnection>{
                ModulePortConnection{ { "leftDelay", "out" },{ "leftDelayHP", "x1" } },
                ModulePortConnection{ { "rightDelay", "out" },{ "rightDelayHP", "x1" } },
                ModulePortConnection{ { "leftDelayHP", "y1" },{ "leftDelayLP", "x1" } },
                ModulePortConnection{ { "rightDelayHP", "y1" },{ "rightDelayLP", "x1" } },
                ModulePortConnection{ { "leftDelayLP", "y1" },{ "rightDelay", "in" } },
                ModulePortConnection{ { "rightDelayLP", "y1" },{ "leftDelay", "in" } },
                ModulePortConnection{ { "leftDelayLP", "y1" },{ "delayGain", "left" } },
                ModulePortConnection{ { "rightDelayLP", "y1" },{ "delayGain", "right" } },
                ModulePortConnection{ { "lfoPhase", "phase" },{ "lfo", "phase" } },
                ModulePortConnection{ { "lfo", "sine" },{ "delayLeftTime", "input" } },
                ModulePortConnection{ { "lfo", "sine" },{ "delayRightTime", "input" } },
                ModulePortConnection{ { "delayLeftTime", "output" },{ "leftDelay", "time" } },
                ModulePortConnection{ { "delayRightTime", "output" },{ "rightDelay", "time" } }
            },
            std::vector<ModulePortValue>{
                ModulePortValue{ "lfoPhase", "freq", 2.0f },
                ModulePortValue{ "delayLeftTime", "shift", 0.22f },
                ModulePortValue{ "delayRightTime", "shift", 0.45f },
                ModulePortValue{ "delayLeftTime", "scale", 0.001f },
                ModulePortValue{ "delayRightTime", "scale", 0.001f },
                ModulePortValue{ "leftDelay", "gain", 0.56f },
                ModulePortValue{ "rightDelay", "gain", 0.62f },
                ModulePortValue{ "leftDelayLP", "wc", 8500.0f },
                ModulePortValue{ "rightDelayLP", "wc", 7500.0f },
                ModulePortValue{ "leftDelayHP", "wc", 350.0f },
                ModulePortValue{ "rightDelayHP", "wc", 450.0f },
                ModulePortValue{ "delayGain", "gain", 1.0f }
            }
        }
    };

    const ComponentDescriptor& getExampleComponent() { return stereoTapeDelay; }

    const ConnectionGraphDescriptor exFxChain = {
        std::vector<ModuleVariable>{
            ModuleVariable{ "outPreGain", "GAIN" },
            ModuleVariable{ "outSaturation", "SSATAN" },
            ModuleVariable{ "outPostGain", "GAIN" },
            ModuleVariable{ "delayGain", "GAIN" },
            ModuleVariable{ "leftDelay", "DELAY" },
            ModuleVariable{ "rightDelay", "DELAY" },
            ModuleVariable{ "lfoPhase", "PHASE" },
            ModuleVariable{ "lfo", "SINE" },
            ModuleVariable{ "delayLeftTime", "SCLSHFT" },
            ModuleVariable{ "delayRightTime", "SCLSHFT" },
            ModuleVariable{ "leftDelayLP", "RCLP" },
            ModuleVariable{ "rightDelayLP", "RCLP" },
            ModuleVariable{ "leftDelayHP", "RCHP" },
            ModuleVariable{ "rightDelayHP", "RCHP" },
        },
        std::vector<ModulePortConnection>{
            ModulePortConnection{ { "inBus", "left" },{ "outPreGain", "left" } },
            ModulePortConnection{ { "inBus", "right" },{ "outPreGain", "right" } },

            ModulePortConnection{ { "outPreGain", "left" },{ "leftDelayHP", "x1" } },
            ModulePortConnection{ { "outPreGain", "right" },{ "rightDelayHP", "x1" } },

            // a cross feedback stereo delay
            ModulePortConnection{ { "leftDelay", "out" },{ "leftDelayHP", "x1" } },
            ModulePortConnection{ { "rightDelay", "out" },{ "rightDelayHP", "x1" } },
            ModulePortConnection{ { "leftDelayHP", "y1" },{ "leftDelayLP", "x1" } },
            ModulePortConnection{ { "rightDelayHP", "y1" },{ "rightDelayLP", "x1" } },
            ModulePortConnection{ { "leftDelayLP", "y1" },{ "rightDelay", "in" } },
            ModulePortConnection{ { "rightDelayLP", "y1" },{ "leftDelay", "in" } },
            // - output and "wet" factor
            ModulePortConnection{ { "leftDelayLP", "y1" },{ "delayGain", "left" } },
            ModulePortConnection{ { "rightDelayLP", "y1" },{ "delayGain", "right" } },
            // - time modulation, causes artifact due to no resampling ...
            ModulePortConnection{ { "lfoPhase", "phase" },{ "lfo", "phase" } },
            ModulePortConnection{ { "lfo", "sine" },{ "delayLeftTime", "input" } },
            ModulePortConnection{ { "lfo", "sine" },{ "delayRightTime", "input" } },
            ModulePortConnection{ { "delayLeftTime", "output" },{ "leftDelay", "time" } },
            ModulePortConnection{ { "delayRightTime", "output" },{ "rightDelay", "time" } },

            ModulePortConnection{ { "delayGain", "left" },{ "outPreGain", "left" } },
            ModulePortConnection{ { "delayGain", "right" },{ "outPreGain", "right" } },

            // saturating gain stage
            ModulePortConnection{ { "outPreGain", "left" },{ "outSaturation", "left" } },
            ModulePortConnection{ { "outPreGain", "right" },{ "outSaturation", "right" } },
            ModulePortConnection{ { "outSaturation", "left" },{ "outPostGain", "left" } },
            ModulePortConnection{ { "outSaturation", "right" },{ "outPostGain", "right" } },
            ModulePortConnection{ { "outPostGain", "left" },{ "outBus", "left" } },
            ModulePortConnection{ { "outPostGain", "right" },{ "outBus", "right" } }
        },
        std::vector<ModulePortValue>{
            ModulePortValue{ "outPreGain", "gain", 0.5f },
            ModulePortValue{ "outPostGain", "gain", 2.0f },

            // delay time moduluation
            ModulePortValue{ "lfoPhase", "freq", 2.0f },
            ModulePortValue{ "delayLeftTime", "shift", 0.22f },
            ModulePortValue{ "delayRightTime", "shift", 0.45f },
            ModulePortValue{ "delayLeftTime", "scale", 0.001f },
            ModulePortValue{ "delayRightTime", "scale", 0.001f },
            // - feedback amount
            ModulePortValue{ "leftDelay", "gain", 0.56f },
            ModulePortValue{ "rightDelay", "gain", 0.62f },
            // - lowpass of feedback loop (sounds good, but also AA ...)
            ModulePortValue{ "leftDelayLP", "wc", 8500.0f },
            ModulePortValue{ "rightDelayLP", "wc", 7500.0f },
            // - lowpass of feeback loop (sounds good, but also AA ...)
            ModulePortValue{ "leftDelayHP", "wc", 350.0f },
            ModulePortValue{ "rightDelayHP", "wc", 450.0f },
            // - effect "wet amount"
            ModulePortValue{ "delayGain", "gain", 1.0f },
        }
    };

    const ConnectionGraphDescriptor& getExampleFxChain() { return exFxChain; }

    const ConnectionGraphDescriptor exVoiceChain = {
        std::vector<ModuleVariable>{
            {"ampEnv", "ENV"},
            {"filtEnv", "ENV"},
            {"clk1", "SCLSHFT"},
            {"clk2", "SCLSHFT"},
            {"ph", "PHASE"},
            {"ph1", "PHASE"},
            {"ph2", "PHASE"},
            {"lfoClk1", "PHASE"},
            {"lfoClk2", "PHASE"},
            {"lfo1", "ABS"},
            {"lfo2", "ABS"},
            {"osc", "SINE"},
            {"sq1", "PBLOSC"},
            {"sq2", "PBLOSC"},
            {"vca", "GAIN"},
            {"lpDesign", "LPF"},
            {"wc", "SCLSHFT"},
            {"snap", "SCLSHFT"},
            {"flt1", "BIQUAD"},
            {"flt2", "BIQUAD"},
            {"ampGain", "MUL"},
        },
        std::vector<ModulePortConnection>{
            { { "inBus", "gate" }, { "ampEnv", "gate" } },
            { { "inBus", "gate" }, { "filtEnv", "gate" } },

            { { "inBus", "pitch_hz" }, { "ph", "freq" } },
            { { "ph", "phase" }, { "osc", "phase" } },

            { { "inBus", "pitch_hz" }, { "clk1", "input" } },
            { { "inBus", "pitch_hz" }, { "clk2", "input" } },
            { { "clk1", "output" }, { "ph1", "freq" } },
            { { "clk2", "output" }, { "ph2", "freq" } },
            { { "ph1", "phase" }, { "sq1", "phase" } },
            { { "ph2", "phase" }, { "sq2", "phase" } },
            { { "inBus", "slide_y" }, { "sq1", "shape" } },
            { { "inBus", "slide_y" }, { "sq2", "shape" } },
            { { "lfoClk1", "phase" }, { "lfo1", "input" } },
            { { "lfoClk2", "phase" }, { "lfo2", "input" } },
            { { "lfo1", "abs" }, { "sq1", "pwm" } },
            { { "lfo2", "abs" }, { "sq2", "pwm" } },

            { { "inBus", "strike_z" }, { "ampEnv", "onBumpHeight" } },
            { { "inBus", "strike_z" }, { "filtEnv", "onBumpHeight" } },
            { { "inBus", "strike_z" }, { "snap", "input" } },

            { { "inBus", "press_z" }, { "ampEnv", "sustainHeight" } },
            { { "inBus", "press_z" }, { "filtEnv", "sustainHeight" } },

            { { "osc", "sine" }, { "vca", "left" } },
            { { "osc", "sine" }, { "vca", "right" } },

            { { "sq1", "output" },{ "flt1", "input" } },
            { { "sq2", "output" },{ "flt2", "input" } },
            { { "flt1", "output" }, { "vca", "left" } },
            { { "flt2", "output" }, { "vca", "right" } },
            { { "ampEnv", "value" }, { "ampGain", "in1" } },
            { { "ampGain", "prod" }, { "vca", "gain" } },
            { { "vca", "left" }, { "outBus", "left" } },
            { { "vca", "right" }, { "outBus", "right" } },

            { { "lpDesign", "a1" }, { "flt1", "a1" } },
            { { "lpDesign", "a2" }, { "flt1", "a2" } },
            { { "lpDesign", "b0" }, { "flt1", "b0" } },
            { { "lpDesign", "b1" }, { "flt1", "b1" } },
            { { "lpDesign", "b2" }, { "flt1", "b2" } },

            { { "lpDesign", "a1" }, { "flt2", "a1" } },
            { { "lpDesign", "a2" }, { "flt2", "a2" } },
            { { "lpDesign", "b0" }, { "flt2", "b0" } },
            { { "lpDesign", "b1" }, { "flt2", "b1" } },
            { { "lpDesign", "b2" }, { "flt2", "b2" } },

            { { "snap", "output" }, { "filtEnv", "onDecaySpeed" } },
            { { "filtEnv", "value" }, { "wc", "input" } },
            { { "wc", "output" }, { "lpDesign", "f0" } },

        },
        std::vector<ModulePortValue>{
            { {"clk1", "scale"}, 1.0f},
            { {"clk1", "shift"}, 0.9f},
            { {"clk2", "scale"}, 1.0f},
            { {"clk2", "shift"}, -1.f},
            { {"lpDesign", "Q"}, 3.f},
            { {"wc", "scale"}, 22000.f },
            { {"wc", "shift"}, 100.f },

            { {"filtEnv", "onBumpHeight"}, 1.f },
            { {"filtEnv", "sustainHeight"}, 0.05f },
            { {"snap", "scale"}, 0.20f},
            { {"snap", "shift"}, 0.05f},
            { {"filtEnv", "offDecaySpeed"}, 0.25f},
            { {"ampEnv", "onDecaySpeed"}, 0.5f},
            { {"ampEnv", "offDecaySpeed"}, 0.5f},

            { {"ampEnv", "offBumpHeight"}, 0.0f},
            { {"filtEnv", "offBumpHeight"}, 0.0f},
            { {"ampEnv", "offAttackSpeed"}, 0.0f},
            { {"filtEnv", "offAttackSpeed"}, 0.0f},
            { {"ampEnv", "sustainHeight"}, 0.0f},
            { {"filtEnv", "sustainHeight"}, 0.0f},

            { {"ampGain", "in2"}, 0.5f},

            { {"lfoClk1", "freq"}, 1.1f},
            { {"lfoClk2", "freq"}, 0.9f},

        }
    };

    const ConnectionGraphDescriptor& getExampleVoiceChain() { return exVoiceChain; }

}
