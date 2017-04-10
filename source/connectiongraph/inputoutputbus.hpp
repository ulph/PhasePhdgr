#ifndef INPUTOUTPUTBUS_HPP
#define INPUTOUTPUTBUS_HPP

#include "module.hpp"

// special modules for the bus
class InputBus : public Module {
    // hooks up all the voice+global inputs and outputs
public:
    InputBus()
    {
        inputs.push_back(Pad("Gate"   )); outputs.push_back(Pad("Gate"   ));
        inputs.push_back(Pad("StrikeZ")); outputs.push_back(Pad("StrikeZ"));
        inputs.push_back(Pad("LiftZ"  )); outputs.push_back(Pad("LiftZ"  ));
        inputs.push_back(Pad("PitchHz")); outputs.push_back(Pad("PitchHz"));
        inputs.push_back(Pad("GlideX" )); outputs.push_back(Pad("GlideX" ));
        inputs.push_back(Pad("SlideY" )); outputs.push_back(Pad("SlideY" ));
        inputs.push_back(Pad("PressZ" )); outputs.push_back(Pad("PressZ" ));
    }

    virtual void process(uint32_t fs)
    {
        for(int i = 0; i < outputs.size(); i++) {
            outputs[i] = inputs[i];
        }
    }
    static Module* factory() { return new InputBus(); }
};

class OutputBus : public Module {
public:
    OutputBus() {
        inputs.push_back(Pad("MonoOut"));
        outputs.push_back(Pad("MonoOut"));
    }

    virtual void process(uint32_t fs)
    {
        outputs[0].value = inputs[0].value;
    }
    static Module* factory() { return new OutputBus(); }
};

#endif
