#pragma once

#include "module.hpp"
#include "design.hpp"

class BusModule : public Module {
private:
    bool isInput;
public:
    BusModule(const vector<PadDescription>& ports, bool isInput)
        : isInput(isInput)
    {
        for(const auto &p : ports){
            const char * name = p.name.c_str();
            const char * unit = p.name.c_str();
            Pad pad(name, p.value, unit);
            inputs.push_back(pad);
            outputs.push_back(pad);
        }
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
