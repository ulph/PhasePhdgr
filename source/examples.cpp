#include "design.hpp"

namespace PhasePhckr {

    const ConnectionGraphDescriptor exFxChain = {
        std::vector<ModuleVariable>{
            { "outPreGain", "GAIN" },
            { "outSaturation", "SSATAN" },
            { "outPostGain", "GAIN" },
            { "stereoTape", "@STEREOTAPE"}
        },
        std::vector<ModulePortConnection>{
            { { "inBus", "left" },{ "outPreGain", "left" } },
            { { "inBus", "right" },{ "outPreGain", "right" } },
            // adding in a Component
            { { "inBus", "left" },{ "stereoTape", "left" } },
            { { "inBus", "right" },{ "stereoTape", "right" } },
            { { "stereoTape", "left" },{ "outPreGain", "left" } },
            { { "stereoTape", "right" },{ "outPreGain", "right" } },
            // saturating gain stage
            { { "outPreGain", "left" },{ "outSaturation", "left" } },
            { { "outPreGain", "right" },{ "outSaturation", "right" } },
            { { "outSaturation", "left" },{ "outPostGain", "left" } },
            { { "outSaturation", "right" },{ "outPostGain", "right" } },
            { { "outPostGain", "left" },{ "outBus", "left" } },
            { { "outPostGain", "right" },{ "outBus", "right" } }
        },
        std::vector<ModulePortValue>{
            { "outPreGain", "gain", 1.0f },
            { "outPostGain", "gain", 1.0f },
            // overriding component defaults ;)
            { "stereoTape", "feedback", 1.0f },
            { "stereoTape", "saturation", 2.5f },
            { "stereoTape", "leftModDepth", 0.002f },
            { "stereoTape", "rightModDepth", 0.002f },
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
