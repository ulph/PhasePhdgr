#pragma once

#include "phasephckr/docs.hpp"

#include "moduleaccessor.hpp"

class BusModule : public ModuleCRTP<BusModule> {
private:
    bool isInput;
    const size_t numPads;
public:
    BusModule(const vector<PhasePhckr::PadDescription>& ports, bool isInput)
        : isInput(isInput)
        , numPads(ports.size())
    {
        auto newName = isInput ? PhasePhckr::c_inBus.type : PhasePhckr::c_outBus.type;
        ModuleAccessor::setName(*this, newName);
        for(const auto &p : ports){
            const char * name = p.name.c_str();
            const char * unit = p.unit.c_str();
            Pad pad(name, p.defaultValue, unit);
            inputs.push_back(pad);
        }
        outputs = inputs;
    }

    virtual void process() {
        for(size_t i = 0; i < numPads; i++) outputs[i].value = inputs[i].value;
    }

    virtual void block_process() {
        for (size_t i = 0; i < numPads; i++) {
            memcpy(outputs[i].values, inputs[i].values, sizeof(float)*ConnectionGraph::k_blockSize);
        }
    }
};
