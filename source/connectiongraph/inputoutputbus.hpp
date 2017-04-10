#ifndef INPUTOUTPUTBUS_HPP
#define INPUTOUTPUTBUS_HPP

#include "module.hpp"

// special modules for the bus
class VoiceInputBus : public Module {
    // hooks up all the voice+global inputs and outputs
public:
    VoiceInputBus()
    {
        inputs.push_back(Pad("Gate"   )); outputs.push_back(Pad("Gate"   ));
        inputs.push_back(Pad("StrikeZ")); outputs.push_back(Pad("StrikeZ"));
        inputs.push_back(Pad("LiftZ"  )); outputs.push_back(Pad("LiftZ"  ));
        inputs.push_back(Pad("PitchHz")); outputs.push_back(Pad("PitchHz"));
        inputs.push_back(Pad("GlideX" )); outputs.push_back(Pad("GlideX" ));
        inputs.push_back(Pad("SlideY" )); outputs.push_back(Pad("SlideY" ));
        inputs.push_back(Pad("PressZ" )); outputs.push_back(Pad("PressZ" ));
        inputs.push_back(Pad("ModWheel")); outputs.push_back(Pad("ModWheel"));
        inputs.push_back(Pad("Expression")); outputs.push_back(Pad("Expression"));
        inputs.push_back(Pad("Breath")); outputs.push_back(Pad("Breath"));
    }

    virtual void process(uint32_t fs)
    {
        for(int i = 0; i < outputs.size(); i++) {
            outputs[i] = inputs[i];
        }
    }
    static Module* factory() { return new VoiceInputBus(); }
};

class VoiceOutputBus : public Module {
public:
    VoiceOutputBus() {
        inputs.push_back(Pad("MonoOut"));
        outputs.push_back(Pad("MonoOut"));
    }

    virtual void process(uint32_t fs)
    {
        outputs[0].value = inputs[0].value;
    }
    static Module* factory() { return new VoiceOutputBus(); }
};

#endif
