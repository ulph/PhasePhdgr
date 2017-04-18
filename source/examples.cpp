#include "design.hpp"

namespace PhasePhckr {

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

            // a cross feedback stereo delay
            ModulePortConnection{ { "outPreGain", "left" },{ "leftDelayHP", "x1" } },
            ModulePortConnection{ { "outPreGain", "right" },{ "rightDelayHP", "x1" } },
            ModulePortConnection{ { "leftDelay", "out" },{ "leftDelayHP", "x1" } },
            ModulePortConnection{ { "rightDelay", "out" },{ "rightDelayHP", "x1" } },
            ModulePortConnection{ { "leftDelayHP", "y1" },{ "leftDelayLP", "x1" } },
            ModulePortConnection{ { "rightDelayHP", "y1" },{ "rightDelayLP", "x1" } },
            ModulePortConnection{ { "leftDelayLP", "y1" },{ "rightDelay", "in" } },
            ModulePortConnection{ { "rightDelayLP", "y1" },{ "leftDelay", "in" } },
            // - output and "wet" factor
            ModulePortConnection{ { "leftDelayLP", "y1" },{ "delayGain", "left" } },
            ModulePortConnection{ { "rightDelayLP", "y1" },{ "delayGain", "right" } },
            ModulePortConnection{ { "delayGain", "left" },{ "outPreGain", "left" } },
            ModulePortConnection{ { "delayGain", "right" },{ "outPreGain", "right" } },
            // - time modulation, causes artifact due to no resampling ...
            ModulePortConnection{ { "lfoPhase", "phase" },{ "lfo", "phase" } },
            ModulePortConnection{ { "lfo", "sine" },{ "delayLeftTime", "input" } },
            ModulePortConnection{ { "lfo", "sine" },{ "delayRightTime", "input" } },
            ModulePortConnection{ { "delayLeftTime", "output" },{ "leftDelay", "time" } },
            ModulePortConnection{ { "delayRightTime", "output" },{ "rightDelay", "time" } },

            // saturating gain stage
            ModulePortConnection{ { "outPreGain", "left" },{ "outSaturation", "left" } },
            ModulePortConnection{ { "outPreGain", "right" },{ "outSaturation", "right" } },
            ModulePortConnection{ { "outSaturation", "left" },{ "outPostGain", "left" } },
            ModulePortConnection{ { "outSaturation", "right" },{ "outPostGain", "right" } },
            ModulePortConnection{ { "outPostGain", "left" },{ "outBus", "left" } },
            ModulePortConnection{ { "outPostGain", "right" },{ "outBus", "right" } }
        },
        std::vector<ModulePortValue>{
            ModulePortValue{ "outPreGain", "gain", 0.1f },
            ModulePortValue{ "outPostGain", "gain", 1.0f },

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
            ModulePortValue{ "delayGain", "gain", 0.5f },
        }
    };

    const ConnectionGraphDescriptor& getExampleFxChain() { return exFxChain; }

    const ConnectionGraphDescriptor_Numerical exMPEVoiceChain = {
        std::vector<ModuleVariable>{
            {"mixGain", "MUL"},
            {"env", "ENV"},
            {"strikeMulExpr", "MUL"},
            {"exprInv", "CINV"},
            {"envDecayTweak", "SCLSHFT"},
            {"phase", "PHASE"},
            {"osc1", "SINE"},
            {"osc2", "SINE"},
            {"osc3", "SINE"},
            {"ySelection", "XFADE"},
            {"inv", "CINV"},
            {"osc2arg", "SATAN"},
            {"osc2argBoost", "MUL"},
            {"osc3arg", "MUL"},
            {"osc23", "XFADE"},
            {"abs", "ABS"},
            {"scl", "SCLSHFT"},
            {"fold", "FOLD"},
            {"foldPreScale", "MUL"},
            {"lp", "RCLP"},
            {"foldPostScale", "MUL"}
        },
        std::vector<ModulePortConnection_Numerical>{
            { { "inBus", 0}, {"env", 0 }},
            {{ "inBus", 1},{ "env", 1 }},
            {{ "inBus", 2},{ "env", 5 }},
            {{ "inBus", 6},{ "env", 4 }},
            {{ "inBus", 8},{ "strikeMulExpr", 0 }},
            {{ "inBus", 1},{ "strikeMulExpr", 1 }},
            {{ "strikeMulExpr", 0},{ "envDecayTweak", 0 }},
            {{ "envDecayTweak", 0},{ "env", 3 }},
            {{ "inBus", 3},{ "phase", 0 }},
            {{ "phase", 0},{ "osc1", 0 }},
            {{ "osc1", 0},{ "mixGain", 0 }},
            {{ "phase", 0},{ "osc2arg", 0 }},
            {{ "env", 0},{ "inv", 0 }},
            {{ "env", 0},{ "ySelection", 0 }},
            {{ "inv", 0},{ "ySelection", 1 }},
            {{ "inBus", 7},{ "ySelection", 2 }},
            {{ "ySelection", 0},{ "osc2argBoost", 0 }},
            {{ "osc2argBoost", 0},{ "osc2arg", 1 }},
            {{ "osc2arg", 0},{ "osc2", 0 }},
            {{ "osc2arg", 0},{ "osc3arg", 0 }},
            {{ "osc3arg", 0},{ "osc3", 0 }},
            {{ "osc2", 0},{ "osc23", 1 }},
            {{ "osc3", 0},{ "osc23", 0 }},
            {{ "inBus", 5},{ "osc23", 2 }},
            {{ "osc23", 0},{ "mixGain", 0 }},
            {{ "phase", 0},{ "abs", 0 }},
            {{ "abs", 0},{ "scl", 0 }},
            {{ "foldPreScale", 0},{ "fold", 2 }},
            {{ "env", 0},{ "foldPreScale", 0 }},
            {{ "scl", 0},{ "fold", 0 }},
            {{ "fold", 0},{ "lp", 0 }},
            {{ "lp", 0},{ "foldPostScale", 0 }},
            {{ "foldPostScale", 0},{ "mixGain", 0 } },
            {{ "env", 0},{ "mixGain", 1 }},
            {{ "mixGain", 0},{ "outBus", 0 }},
            {{ "mixGain", 0},{ "outBus", 1 }}
        },
        std::vector<ModulePortValue_Numerical>{
            { {"foldPostScale", 1}, 0.25f},
            { {"foldPreScale", 1}, 4.0f},
            { {"osc3arg", 1}, 2.0f},
            { {"osc2argBoost", 1}, 20.0f},
            { {"envDecayTweak", 1}, 2.0f},
            { {"envDecayTweak", 2}, 0.1f}
        }
    };

    const ConnectionGraphDescriptor_Numerical& getExampleMPEVoiceChain() { return exMPEVoiceChain; }

    const ConnectionGraphDescriptor exVoiceChain = {
        std::vector<ModuleVariable>{
            {"ampEnv", "ENV"},
            {"filtEnv", "ENV"},
            {"clk1", "SCLSHFT"},
            {"clk2", "SCLSHFT"},
            {"ph", "PHASE"},
            {"osc", "SINE"},
            {"sq1", "PBLOSC"},
            {"sq2", "PBLOSC"},
            {"vca", "GAIN"},
            {"lpDesign", "LPF"},
            {"wc", "SCLSHFT"},
            {"snap", "SCLSHFT"},
            {"flt1", "BIQUAD"},
            {"flt2", "BIQUAD"},
        },
        std::vector<ModulePortConnection>{
            { { "inBus", "gate" }, { "ampEnv", "gate" } },
            { { "inBus", "gate" }, { "filtEnv", "gate" } },

            { { "inBus", "pitch_hz" }, { "ph", "freq" } },
            { { "ph", "phase" }, { "osc", "phase" } },

            { { "inBus", "pitch_hz" }, { "clk1", "input" } },
            { { "inBus", "pitch_hz" }, { "clk2", "input" } },
            { { "clk1", "output" }, { "sq1", "freq" } },
            { { "clk2", "output" }, { "sq2", "freq" } },
            { { "inBus", "slide_y" }, { "sq1", "shape" } },
            { { "inBus", "slide_y" }, { "sq2", "shape" } },
            { { "inBus", "modwheel" }, { "sq1", "pwm" } },
            { { "inBus", "modwheel" }, { "sq2", "pwm" } },

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
            { { "vca", "left" }, { "outBus", "left" } },
            { { "vca", "right" }, { "outBus", "right" } },
            { { "ampEnv", "value" }, { "vca", "gain" } },

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

            { { "ampEnv", "value" }, { "vca", "gain" } },

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

        }
    };

    const ConnectionGraphDescriptor& getExampleVoiceChain() { return exVoiceChain; }

}
