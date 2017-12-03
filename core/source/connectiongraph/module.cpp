#include "module.hpp"

void Module::block_process(uint32_t fs) {
    const size_t inputsSize = inputs.size();
    const size_t outputsSize = outputs.size();
    for (int i = 0; i < ConnectionGraph::k_blockSize; ++i) {
        for (int k = 0; k < inputsSize; ++k) {
            unbuffer_set_input(k, i);
        }
        process(fs);
        for (int k = 0; k < outputsSize; ++k) {
            buffer_set_output(k, i);
        }
    }
}

int Module::getInputPadFromName(std::string padName) const {
    for (int i = 0; i < (int)inputs.size(); i++) {
        if (inputs[i].name == padName) {
            return i;
        }
    }
    return -1;
}

int Module::getOutputPadFromName(std::string padName) const {
    for (int i = 0; i < (int)outputs.size(); i++) {
        if (outputs[i].name == padName) {
            return i;
        }
    }
    return -1;
}

void Module::setName(const std::string &n) { name = n; }

std::string Module::docString() const { return "..."; }

PhasePhckr::ModuleDoc Module::makeDoc() const {
    PhasePhckr::ModuleDoc doc;
    doc.type = name;
    doc.docString = docString();
    for (const auto p : inputs) {
        PhasePhckr::PadDescription pd;
        pd.name = p.name;
        pd.unit = p.unit;
        pd.defaultValue = p.value;
        doc.inputs.push_back(pd);
    }
    for (const auto p : outputs) {
        PhasePhckr::PadDescription pd;
        pd.name = p.name;
        pd.unit = p.unit;
        pd.defaultValue = p.value;
        doc.outputs.push_back(pd);
    }
    return doc;
}