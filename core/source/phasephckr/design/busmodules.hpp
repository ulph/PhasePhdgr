#pragma once

#include <phasephckr.hpp>

#include "module.hpp"

class BusModule : public ModuleCRTP<BusModule> {
private:
    bool isInput;
public:
    BusModule(const vector<PadDescription>& ports, bool isInput)
        : isInput(isInput)
    {
        if(isInput){
            name = PhasePhckr::c_inBus.type;
        }
        else {
            name = PhasePhckr::c_outBus.type;
        }
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
        for(int i = 0; i < (int)outputs.size(); i++) {
            outputs[i].value = inputs[i].value;
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
