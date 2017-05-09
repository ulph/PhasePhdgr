#pragma once

#include "module.hpp"
#include "design.hpp"

class BusModule : public Module {
private:
    bool isInput;
public:
    BusModule(const vector<Pad>& ports, bool isInput)
        : isInput(isInput)
    {
        inputs = ports;
        outputs = ports;
    }
    virtual void process(uint32_t fs)
    {
        for(int i = 0; i < outputs.size(); i++) {
            outputs[i] = inputs[i];
        }
    }
    virtual ModuleDoc makeDoc() {
        auto d = Module::makeDoc();
        if(isInput)
            d.inputs.clear();
        else
            d.outputs.clear();
        return d;
    }
};
