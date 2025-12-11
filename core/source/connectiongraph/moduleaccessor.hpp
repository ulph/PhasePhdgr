#pragma once

#include <cstring>
#include <iostream>
#include "module.hpp"

class ModuleAccessor {
   public:
    static void processSample(Module& m, int sample) { m.processSample(sample); }
    static void processBlock(Module& m) { m.processBlock(); }

    static void reset(Module& m) {
        for (auto& p : m.inputs)
            p.reset();
        for (auto& p : m.outputs)
            p.reset();
    }

    static const std::vector<Pad>& getInputs(const Module& m) { return m.inputs; }
    static const std::vector<Pad>& getOutputs(const Module& m) { return m.outputs; }

    static void setName(Module& m, const std::string& n) { m.name = n; }
    static const std::string& getName(const Module& m) { return m.name; }

    static int getNumInputPads(const Module& m) { return (int)m.inputs.size(); }
    static int getNumOutputPads(const Module& m) { return (int)m.outputs.size(); }
    static int getInputPadFromName(const Module& m, std::string padName) {
        for (int i = 0; i < (int)m.inputs.size(); i++) {
            if (m.inputs[i].name == padName) {
                return i;
            }
        }
        std::cerr << "Error: Module '" << m.name << "' has no input pad with name '" << padName
                  << "'" << std::endl;
        return -1;
    }
    static int getOutputPadFromName(const Module& m, std::string padName) {
        for (int i = 0; i < (int)m.outputs.size(); i++) {
            if (m.outputs[i].name == padName) {
                return i;
            }
        }
        std::cerr << "Error: Module '" << m.name << "' has no output pad with name '" << padName
                  << "'" << std::endl;
        return -1;
    }

    static void setFs(Module& m, float newFs) {
        m.fs = newFs;
        m.fsInv = 1.f / m.fs;
        m.init();
    }

    static void setSampleOutputToInput(const Module& fromModule,
                                       int fromPad,
                                       int fromSample,
                                       Module& toModule,
                                       int toPad,
                                       int toSample) {
        toModule.inputs[toPad].values[toSample] = fromModule.outputs[fromPad].values[fromSample];
    }
    static void addSampleOutputToInput(const Module& fromModule,
                                       int fromPad,
                                       int fromSample,
                                       Module& toModule,
                                       int toPad,
                                       int toSample) {
        toModule.inputs[toPad].values[toSample] += fromModule.outputs[fromPad].values[fromSample];
    }

    static void getOutput(const Module& m, int outputPad, float* buffer) {
        memcpy(buffer, m.outputs[outputPad].values, sizeof(float) * Pad::k_blockSize);
    }
    static void setInputFromSample(Module& m, int inputPad, float value) {
        for (int i = 0; i < Pad::k_blockSize; ++i) {
            m.inputs[inputPad].values[i] = value;
        }
    }
    static void setInput(Module& m, int inputPad, const float* buffer) {
        memcpy(m.inputs[inputPad].values, buffer, sizeof(float) * Pad::k_blockSize);
    }
    static void setBlockOutputToInput(const Module& fromModule,
                                      int fromPad,
                                      Module& toModule,
                                      int toPad) {
        memcpy(toModule.inputs[toPad].values, fromModule.outputs[fromPad].values,
               sizeof(toModule.inputs[toPad].values));
    }
    static void addBlockOutputToInput(const Module& fromModule,
                                      int fromPad,
                                      Module& toModule,
                                      int toPad) {
        for (int sample = 0; sample < Pad::k_blockSize; ++sample) {
            toModule.inputs[toPad].values[sample] += fromModule.outputs[fromPad].values[sample];
        }
    }
};
