#pragma once

#include "module.hpp"

class ModuleAccessor
{
    // Anti-pattern to break out all the non-sdk stuff ...

public:
    static void process(Module& m) {
        m.process();
    }
    static void block_process(Module& m) {
        m.block_process();
    }

    static const std::vector<Pad>& getInputs(const Module& m) { return m.inputs; }
    static const std::vector<Pad>& getOutputs(const Module& m) { return m.outputs; }

    static void setName(Module& m, const std::string &n) { m.name = n; }
    static const std::string & getName(const Module& m) { return m.name; }

    static int getNumInputPads(const Module& m) { return (int)m.inputs.size(); }
    static int getNumOutputPads(const Module& m) { return (int)m.outputs.size(); }
    static int getInputPadFromName(const Module& m, std::string padName) {
        for (int i = 0; i < (int)m.inputs.size(); i++) {
            if (m.inputs[i].name == padName) {
                return i;
            }
        }
        std::cerr << "Error: Module '" << m.name << "' has no input pad with name '" << padName << "'" << std::endl;
        return -1;
    }
    static int getOutputPadFromName(const Module& m, std::string padName) {
        for (int i = 0; i < (int)m.outputs.size(); i++) {
            if (m.outputs[i].name == padName) {
                return i;
            }
        }
        std::cerr << "Error: Module '" << m.name << "' has no output pad with name '" << padName << "'" << std::endl;
        return -1;
    }

    static void setFs(Module& m, float newFs) {
        m.fs = newFs;
        m.fsInv = 1.f / m.fs;
        m.init();
    }

    static void setInput(Module& m, int inputPad, float value) {
        sample_setInput(m, inputPad, value);
        block_fillInput(m, inputPad, value);
    }

    // sample processing
    static float sample_getOutput(const Module& m, int outputPad) {
        return m.outputs[outputPad].value;
    }
    static void sample_setInput(Module& m, int inputPad, float value) {
        m.inputs[inputPad].value = value;
    }
    static void sample_resetInput(Module& m, int inputPad) {
        m.inputs[inputPad].value = 0.0f;
    }
    static void sample_addToInput(Module& m, int inputPad, float value) {
        m.inputs[inputPad].value += value;
    }

    // sample to block helpers
    static void unbuffer_add_input(Module& m, int inputPad, int i) {
        m.inputs[inputPad].value += m.inputs[inputPad].values[i];
    }
    static void buffer_set_output(Module& m, int outputPad, int i) {
        m.outputs[outputPad].values[i] = m.outputs[outputPad].value;
    }

    // block processing
    static void block_getOutput(const Module& m, int outputPad, float* buffer) {
        memcpy(buffer, m.outputs[outputPad].values, sizeof(float)*Pad::k_blockSize);
    }
    static void block_fillInput(Module& m, int inputPad, float value) {
        for (int i = 0; i < Pad::k_blockSize; ++i) {
            m.inputs[inputPad].values[i] = value;
        }
    }
    static void block_setInput(Module& m, int inputPad, const float* buffer) {
        memcpy(m.inputs[inputPad].values, buffer, sizeof(float)*Pad::k_blockSize);
    }
    static void block_addToInput(Module& m, int inputPad, const float* buffer) {
        for (int i = 0; i < Pad::k_blockSize; ++i) {
            auto v = m.inputs[inputPad].values[i];
            v += buffer[i];
            m.inputs[inputPad].values[i] = v;
        }
    }
};
