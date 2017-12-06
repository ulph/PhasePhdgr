#include "phasephckr/examples.hpp"

namespace PhasePhckr {

    const ConnectionGraphDescriptor exFxChain = {
        std::map<string, string>{
            { "outPreGain", "GAIN" },
            { "outSaturation", "SSATAN" },
            { "outPostGain", "GAIN" },
            { "delayAmount", "CINV"},
            { "delayGain", "GAIN"},
            { "stereoTape", "@STEREOTAPE"}
        },
        std::vector<ModulePortConnection>{
            { { "inBus", "left" },{ "outPreGain", "left" } },
            { { "inBus", "right" },{ "outPreGain", "right" } },
            // adding in a Component
            { { "inBus", "left" },{ "delayGain", "left" } },
            { { "inBus", "right" },{ "delayGain", "right" } },
            { { "delayGain", "left" },{ "stereoTape", "left" } },
            { { "delayGain", "right" },{ "stereoTape", "right" } },
            { { "stereoTape", "left" },{ "outPreGain", "left" } },
            { { "stereoTape", "right" },{ "outPreGain", "right" } },
            // scale the component output via modwheel
            { { "inBus", "mod" },{ "delayAmount", "input" } },
            { { "delayAmount", "output" },{ "delayGain", "gain" } },
            // saturating gain stage
            { { "outPreGain", "left" },{ "outSaturation", "left" } },
            { { "outPreGain", "right" },{ "outSaturation", "right" } },
            { { "outSaturation", "left" },{ "outPostGain", "left" } },
            { { "outSaturation", "right" },{ "outPostGain", "right" } },
            { { "outPostGain", "left" },{ "outBus", "left" } },
            { { "outPostGain", "right" },{ "outBus", "right" } }
        },
        std::map<ModulePort, float>{
            { {"outPreGain", "gain"}, 0.5f },
            { {"outPostGain", "gain"}, 1.0f },
            // overriding component defaults ;)
            { {"stereoTape", "feedback"}, 0.75f },
            { {"stereoTape", "saturation"}, 1.0f },
            { {"stereoTape", "leftModDepth"}, 0.002f },
            { {"stereoTape", "rightModDepth"}, 0.002f },
        }
    };

    PatchDescriptor getExampleEffectChain() {
        PatchDescriptor ex;
        ex.root.graph = exFxChain;
        return ex;
    }

    const ConnectionGraphDescriptor exVoiceChain = {
        std::map<string, string>{
            {"ampEnv", "ENV"},
            {"filtEnv", "ENV"},
            {"clk1", "SCLSHFT"},
            {"clk2", "SCLSHFT"},
            {"ph", "PHASE"},
            {"lfoClk1", "PHASE"},
            {"lfoClk2", "PHASE"},
            {"lfo1", "ABS"},
            {"lfo2", "ABS"},
            {"osc", "SINE"},
            {"sq1", "PBLOSC"},
            {"sq2", "PBLOSC"},
            {"vca", "GAIN"},
            {"lpDesign", "BQLPF"},
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
            { { "clk1", "output" }, { "sq1", "freq" } },
            { { "clk2", "output" }, { "sq2", "freq" } },
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
        std::map<ModulePort, float>{
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

    PatchDescriptor getExampleVoiceChain() { 
        PatchDescriptor ex;
        ex.root.graph = exVoiceChain;
        return ex;
    }

}
