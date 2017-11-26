#ifndef MODULE_HPP
#define MODULE_HPP

#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <vector>
#include <string>

#include "connectiongraph.hpp"

#include "phasephckr/docs.hpp"

struct Pad
{
private:
    void init() { for (int i = 0; i < ConnectionGraph::k_blockSize; ++i) values[i] = value; }
public:
    std::string name;
    float value;
    float values[ConnectionGraph::k_blockSize];
    std::string unit;
    Pad(const char *name) : name(name), value(0.0f), unit("") { init(); }
    Pad(const char *name, float value) : name(name), value(value), unit("") { init(); }
    Pad(const char *name, float value, const char *unit) : name(name), value(value), unit(unit) { init(); }
    Pad(const char *name, const char *unit) : name(name), value(0.0f), unit(unit) { init(); }
};

class Module
{
protected:
    std::string name;
    std::vector<Pad> inputs;
    std::vector<Pad> outputs;

public:
    virtual ~Module() {}
    virtual Module *clone() const = 0;

    virtual void process(uint32_t fs) = 0;

    float sample_getOutput(int outputPad) const {
        return outputs[outputPad].value;
    }

    void setInput(int inputPad, float value) {
        inputs[inputPad].value = value;
        // tmp hack, do both
        for (int i = 0; i < ConnectionGraph::k_blockSize; ++i) {
            inputs[inputPad].values[i] = value;
        }
    }

    void sample_resetInput(int inputPad) {
        inputs[inputPad].value = 0.0f;
    }

    void sample_addToInput(int inputPad, float value) {
        inputs[inputPad].value += value;
    }

    virtual void block_process(uint32_t fs) {
        for (int i = 0; i < ConnectionGraph::k_blockSize; ++i) {
            // tmp hack, use sample processing copying values back and forth
            for (int k = 0; k < inputs.size(); ++k) {
                inputs[k].value = inputs[k].values[i];
            }
            process(fs);
            for (int k = 0; k < outputs.size(); ++k) {
                outputs[k].values[i] = outputs[k].value;
            }
        }
    }

    void block_getOutput(int outputPad, float* buffer) const {
        for (int i = 0; i < ConnectionGraph::k_blockSize; ++i) {
            buffer[i] = outputs[outputPad].values[i];
        }
    }

    void block_resetInput(int inputPad) {
        for (int i = 0; i < ConnectionGraph::k_blockSize; ++i) {
            inputs[inputPad].values[i] = 0.0f;
        }
    }

    void block_addToInput(int inputPad, const float* buffer) {
        for (int i = 0; i < ConnectionGraph::k_blockSize; ++i) {
            inputs[inputPad].values[i] += buffer[i];
        }
    }

    int getNumInputPads() const { return (int)inputs.size(); }

    int getNumOutputPads() const { return (int)outputs.size(); }
    
    int getInputPadFromName(std::string padName) const {
        for(int i = 0; i < (int)inputs.size(); i++) {
            if(inputs[i].name == padName) {
                return i;
            }
        }
        return -1;
    }
    
    int getOutputPadFromName(std::string padName) const {
        for(int i = 0; i < (int)outputs.size(); i++) {
            if(outputs[i].name == padName) {
                return i;
            }
        }
        return -1;
    }

    void setName(const std::string &n) { name = n; }

    std::string getName() const { return name; }

    virtual std::string docString() const { return "..."; }

    virtual PhasePhckr::ModuleDoc makeDoc() const {
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

};

// CRTP pattern
template <class D>
class ModuleCRTP : public Module {
public:
  virtual Module *clone() const {
      return new D(static_cast<D const&>(*this));
  }
};


#endif
