#pragma once

#include "phasephckr/docs.hpp"

#include "module.hpp"

class BusModule : public ModuleCRTP<BusModule> {
private:
    bool isInput;
    const size_t numPads;
public:
    BusModule(const vector<PhasePhckr::PadDescription>& ports, bool isInput)
        : isInput(isInput)
        , numPads(ports.size())
    {
        name = isInput ? PhasePhckr::c_inBus.type : PhasePhckr::c_outBus.type;
        for(const auto &p : ports){
            const char * name = p.name.c_str();
            const char * unit = p.name.c_str();
            Pad pad(name, p.defaultValue, unit);
            inputs.push_back(pad);
        }
        outputs = inputs;
    }

    virtual void process(uint32_t fs) {
        for(size_t i = 0; i < numPads; i++) outputs[i].value = inputs[i].value;
    }

    virtual PhasePhckr::ModuleDoc makeDoc() {
        auto d = Module::makeDoc();
        if(isInput)
            d.inputs.clear();
        else
            d.outputs.clear();
        return d;
    }
};
