#include "phasephckr/examples.hpp"

namespace PhasePhckr {

    const ConnectionGraphDescriptor exFxChain = {
        std::map<string, string>{
            { "outPreGain", "GAIN" },
            { "outSaturation", "SNATAN" },
            { "outPostGain", "GAIN" },
            { "delayAmount", "CLAMPINV"},
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
            { { "inBus", "mod" },{ "delayAmount", "in" } },
            { { "delayAmount", "out" },{ "delayGain", "gain" } },
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
            {"ampEnv", "CAMELENV"},
            {"filtEnv", "CAMELENV"},
            {"clk1", "MULADD"},
            {"clk2", "MULADD"},
            {"ph", "PHASE"},
            {"lfoClk1", "PHASE"},
            {"lfoClk2", "PHASE"},
            {"lfo1", "ABS"},
            {"lfo2", "ABS"},
            {"osc", "SINE"},
            {"sq1", "OSC_BLIT"},
            {"sq2", "OSC_BLIT"},
            {"vca", "GAIN"},
            {"lpDesign", "BQ_LP"},
            {"wc", "MULADD"},
            {"snap", "MULADD"},
            {"flt1", "BQ_FILTER"},
            {"flt2", "BQ_FILTER"},
            {"ampGain", "MUL"},
        },
        std::vector<ModulePortConnection>{
            { { "inBus", "gate" }, { "ampEnv", "gate" } },
            { { "inBus", "gate" }, { "filtEnv", "gate" } },

            { { "inBus", "pitch_hz" }, { "ph", "freq" } },
            { { "ph", "phase" }, { "osc", "phase" } },

            { { "inBus", "pitch_hz" }, { "clk1", "in" } },
            { { "inBus", "pitch_hz" }, { "clk2", "in" } },
            { { "clk1", "out" }, { "sq1", "freq" } },
            { { "clk2", "out" }, { "sq2", "freq" } },
            { { "inBus", "slide_y" }, { "sq1", "shape" } },
            { { "inBus", "slide_y" }, { "sq2", "shape" } },
            { { "lfoClk1", "phase" }, { "lfo1", "in" } },
            { { "lfoClk2", "phase" }, { "lfo2", "in" } },
            { { "lfo1", "out" }, { "sq1", "pwm" } },
            { { "lfo2", "out" }, { "sq2", "pwm" } },

            { { "inBus", "strike_z" }, { "ampEnv", "onBumpHeight" } },
            { { "inBus", "strike_z" }, { "filtEnv", "onBumpHeight" } },
            { { "inBus", "strike_z" }, { "snap", "in" } },

            { { "inBus", "press_z" }, { "ampEnv", "sustainHeight" } },
            { { "inBus", "press_z" }, { "filtEnv", "sustainHeight" } },

            { { "osc", "sine" }, { "vca", "left" } },
            { { "osc", "sine" }, { "vca", "right" } },

            { { "sq1", "out" },{ "flt1", "in" } },
            { { "sq2", "out" },{ "flt2", "in" } },
            { { "flt1", "out" }, { "vca", "left" } },
            { { "flt2", "out" }, { "vca", "right" } },
            { { "ampEnv", "out" }, { "ampGain", "in" } },
            { { "ampGain", "out" }, { "vca", "gain" } },
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

            { { "snap", "out" }, { "filtEnv", "onDecaySpeed" } },
            { { "filtEnv", "out" }, { "wc", "in" } },
            { { "wc", "out" }, { "lpDesign", "fc" } },

        },
        std::map<ModulePort, float>{
            { {"clk1", "mul"}, 1.0f},
            { {"clk1", "add"}, 0.9f},
            { {"clk2", "mul"}, 1.0f},
            { {"clk2", "add"}, -1.f},
            { {"lpDesign", "Q"}, 3.f},
            { {"wc", "mul"}, 22000.f },
            { {"wc", "add"}, 100.f },

            { {"filtEnv", "onBumpHeight"}, 1.f },
            { {"filtEnv", "sustainHeight"}, 0.05f },
            { {"snap", "mul"}, 0.20f},
            { {"snap", "add"}, 0.05f},
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
