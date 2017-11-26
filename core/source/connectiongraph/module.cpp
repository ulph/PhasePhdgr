#include "module.hpp"

void Module::setInput(int inputPad, float value) {
    sample_setInput(inputPad, value);
    block_setInput(inputPad, value);
}

float Module::sample_getOutput(int outputPad) const {
    return outputs[outputPad].value;
}

void Module::sample_setInput(int inputPad, float value) {
    inputs[inputPad].value = value;
}

void Module::sample_resetInput(int inputPad) {
    inputs[inputPad].value = 0.0f;
}

void Module::sample_addToInput(int inputPad, float value) {
    inputs[inputPad].value += value;
}

void Module::block_process(uint32_t fs) {
    const size_t inputsSize = inputs.size();
    const size_t outputsSize = outputs.size();
    for (int i = 0; i < ConnectionGraph::k_blockSize; ++i) {
        for (int k = 0; k < inputsSize; ++k) {
            inputs[k].value = inputs[k].values[i];
        }
        process(fs);
        for (int k = 0; k < outputsSize; ++k) {
            outputs[k].values[i] = outputs[k].value;
        }
    }
}

void Module::block_getOutput(int outputPad, float* buffer) const {
    memcpy(buffer, outputs[outputPad].values, sizeof(float)*ConnectionGraph::k_blockSize);
}

void Module::block_setInput(int inputPad, float value) {
    for (int i = 0; i < ConnectionGraph::k_blockSize; ++i) {
        inputs[inputPad].values[i] = value;
    }
}

void Module::block_setInput(int inputPad, const float* buffer) {
    memcpy(outputs[inputPad].values, buffer, sizeof(float)*ConnectionGraph::k_blockSize);
}

void Module::block_resetInput(int inputPad) {
    for (int i = 0; i < ConnectionGraph::k_blockSize; ++i) {
        inputs[inputPad].values[i] = 0.0f;
    }
}

void Module::block_addToInput(int inputPad, const float* buffer) {
    for (int i = 0; i < ConnectionGraph::k_blockSize; ++i) {
        auto v = inputs[inputPad].values[i];
        v += buffer[i];
        inputs[inputPad].values[i] = v;
    }
}

int Module::getNumInputPads() const { return (int)inputs.size(); }

int Module::getNumOutputPads() const { return (int)outputs.size(); }

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