#pragma once

#include "module.hpp"

class VoiceInputBus : public Module {
public:
    VoiceInputBus()
    {
        inputs.push_back(Pad("Gate"      )); outputs.push_back(Pad("Gate"      ));
        inputs.push_back(Pad("StrikeZ"   )); outputs.push_back(Pad("StrikeZ"   ));
        inputs.push_back(Pad("LiftZ"     )); outputs.push_back(Pad("LiftZ"     ));
        inputs.push_back(Pad("PitchHz"   )); outputs.push_back(Pad("PitchHz"   ));
        inputs.push_back(Pad("GlideX"    )); outputs.push_back(Pad("GlideX"    ));
        inputs.push_back(Pad("SlideY"    )); outputs.push_back(Pad("SlideY"    ));
        inputs.push_back(Pad("PressZ"    )); outputs.push_back(Pad("PressZ"    ));
        inputs.push_back(Pad("ModWheel"  )); outputs.push_back(Pad("ModWheel"  ));
        inputs.push_back(Pad("Expression")); outputs.push_back(Pad("Expression"));
        inputs.push_back(Pad("Breath"    )); outputs.push_back(Pad("Breath"    ));
    }

    virtual void process(uint32_t fs)
    {
        for(int i = 0; i < outputs.size(); i++) {
            outputs[i] = inputs[i];
        }
    }
    static Module* factory() { return new VoiceInputBus(); }
};

class StereoBus : public Module {
public:
    StereoBus() {
        inputs.push_back(Pad("Left"));
        inputs.push_back(Pad("Right"));
        outputs.push_back(Pad("Left"));
        outputs.push_back(Pad("Right"));
    }

    virtual void process(uint32_t fs)
    {
        outputs[0].value = inputs[0].value;
        outputs[1].value = inputs[1].value;
    }
    static Module* factory() { return new StereoBus(); }
};
