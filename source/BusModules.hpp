#pragma once

#include "module.hpp"

class VoiceInputBus : public Module {
public:
    VoiceInputBus()
    {
        inputs.push_back(Pad("gate"      )); outputs.push_back(Pad("gate"      ));
        inputs.push_back(Pad("strike_z"   )); outputs.push_back(Pad("strike_z"   ));
        inputs.push_back(Pad("lift_z"     )); outputs.push_back(Pad("lift_z"     ));
        inputs.push_back(Pad("pitch_hz"   )); outputs.push_back(Pad("pitch_hz"   ));
        inputs.push_back(Pad("glide_x"    )); outputs.push_back(Pad("glide_x"    ));
        inputs.push_back(Pad("slide_y"    )); outputs.push_back(Pad("slide_y"    ));
        inputs.push_back(Pad("press_z"    )); outputs.push_back(Pad("press_z"    ));
        inputs.push_back(Pad("modwheel"  )); outputs.push_back(Pad("modwheel"  ));
        inputs.push_back(Pad("expression")); outputs.push_back(Pad("expression"));
        inputs.push_back(Pad("breath"    )); outputs.push_back(Pad("breath"    ));
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
        inputs.push_back(Pad("left"      )); outputs.push_back(Pad("left"      ));
        inputs.push_back(Pad("right"     )); outputs.push_back(Pad("right"     ));
    }

    virtual void process(uint32_t fs)
    {
        outputs[0].value = inputs[0].value;
        outputs[1].value = inputs[1].value;
    }
    static Module* factory() { return new StereoBus(); }
};

class EffectInputBus : public Module {
public:
    EffectInputBus() {
        inputs.push_back(Pad("left"      )); outputs.push_back(Pad("left"      ));
        inputs.push_back(Pad("right"     )); outputs.push_back(Pad("right"     ));
        inputs.push_back(Pad("modwheel"  )); outputs.push_back(Pad("modWheel"  ));
        inputs.push_back(Pad("expression")); outputs.push_back(Pad("expression"));
        inputs.push_back(Pad("breath"    )); outputs.push_back(Pad("breath"    ));
    }

    virtual void process(uint32_t fs)
    {
        outputs[0].value = inputs[0].value;
        outputs[1].value = inputs[1].value;
    }
    static Module* factory() { return new EffectInputBus(); }
};
