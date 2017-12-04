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
    float value = 0.0f;
    float values[ConnectionGraph::k_blockSize] = { 0.0f };
    std::string unit = "";
    Pad(const char *name) : name(name) { init(); }
    Pad(const char *name, float value) : name(name), value(value) { init(); }
    Pad(const char *name, float value, const char *unit) : name(name), value(value), unit(unit) { init(); }
    Pad(const char *name, const char *unit) : name(name), unit(unit) { init(); }
};

class Module
{
    friend ConnectionGraph;

protected:
    std::vector<Pad> inputs;
    std::vector<Pad> outputs;
    void setName(const std::string &n);

public:
    virtual ~Module() {}
    virtual Module *clone() const = 0;
    void setInput(int inputPad, float value) {
        sample_setInput(inputPad, value);
        block_fillInput(inputPad, value);
    }
    virtual PhasePhckr::ModuleDoc makeDoc() const;
    virtual std::string docString() const;
    int getNumInputPads() const { return (int)inputs.size(); }
    int getNumOutputPads() const { return (int)outputs.size(); }
    int getInputPadFromName(std::string padName) const;
    int getOutputPadFromName(std::string padName) const;

private:
    std::string name;

    // sample processing
    virtual void process(uint32_t fs) = 0;
    float sample_getOutput(int outputPad) const {
        return outputs[outputPad].value;
    }
    void sample_setInput(int inputPad, float value) {
        inputs[inputPad].value = value;
    }
    void sample_resetInput(int inputPad) {
        inputs[inputPad].value = 0.0f;
    }
    void sample_addToInput(int inputPad, float value) {
        inputs[inputPad].value += value;
    }


    // sample to block helpers
    void unbuffer_set_input(int inputPad, int i) {
        inputs[inputPad].value = inputs[inputPad].values[i];
    }
    void unbuffer_add_input(int inputPad, int i) {
        inputs[inputPad].value += inputs[inputPad].values[i];
    }
    void unbuffer_clear(int inputPad, int i) {
        inputs[inputPad].values[i] = 0.0f;
    }
    void buffer_clear(int outputPad, int i) {
        outputs[outputPad].values[i] = 0.0f;
    }
    void buffer_set_output(int outputPad, int i) {
        outputs[outputPad].values[i] = outputs[outputPad].value;
    }
    void buffer_add_output(int outputPad, int i) {
        outputs[outputPad].values[i] += outputs[outputPad].value;
    }

    // block processing
    virtual void block_process(uint32_t fs);
    void block_getOutput(int outputPad, float* buffer) const {
        memcpy(buffer, outputs[outputPad].values, sizeof(float)*ConnectionGraph::k_blockSize);
    }
    void block_fillInput(int inputPad, float value) {
        for (int i = 0; i < ConnectionGraph::k_blockSize; ++i) {
            inputs[inputPad].values[i] = value;
        }
    }
    void block_setInput(int inputPad, const float* buffer) {
        memcpy(inputs[inputPad].values, buffer, sizeof(float)*ConnectionGraph::k_blockSize);
    }
    void block_resetInput(int inputPad) {
        for (int i = 0; i < ConnectionGraph::k_blockSize; ++i) {
            inputs[inputPad].values[i] = 0.0f;
        }
    }
    void block_addToInput(int inputPad, const float* buffer) {
        for (int i = 0; i < ConnectionGraph::k_blockSize; ++i) {
            auto v = inputs[inputPad].values[i];
            v += buffer[i];
            inputs[inputPad].values[i] = v;
        }
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
