#ifndef MODULE_HPP
#define MODULE_HPP

#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <vector>
#include <string>
#include <string.h>

#include "phasephckr/docs.hpp"

struct Pad
{
private:
    void init() { for (int i = 0; i < k_blockSize; ++i) values[i] = value; }
public:
    static const int k_blockSize = 64;
    std::string name;
    float value = 0.0f;
    float values[Pad::k_blockSize] = { 0.0f };
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
    float fs = 48000.f;
    float fsInv = 1.f / fs;
    virtual void init() {};

public:
    virtual ~Module() {}
    virtual Module *clone() const = 0;
    virtual PhasePhckr::ModuleDoc makeDoc() const; // TODO, hide
    virtual std::string docString() const;

private:
    std::string name = "";

    int getNumInputPads() const { return (int)inputs.size(); }
    int getNumOutputPads() const { return (int)outputs.size(); }
    int getInputPadFromName(std::string padName) const;
    int getOutputPadFromName(std::string padName) const;

    virtual void setFs(float newFs) {
        fs = newFs;
        fsInv = 1.f / fs;
        init();
    }

    void setInput(int inputPad, float value) {
        sample_setInput(inputPad, value);
        block_fillInput(inputPad, value);
    }

    // sample processing
    virtual void process() = 0;
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
    void unbuffer_add_input(int inputPad, int i) {
        inputs[inputPad].value += inputs[inputPad].values[i];
    }
    void buffer_set_output(int outputPad, int i) {
        outputs[outputPad].values[i] = outputs[outputPad].value;
    }

    // block processing
    virtual void block_process();
    void block_getOutput(int outputPad, float* buffer) const {
        memcpy(buffer, outputs[outputPad].values, sizeof(float)*Pad::k_blockSize);
    }
    void block_fillInput(int inputPad, float value) {
        for (int i = 0; i < Pad::k_blockSize; ++i) {
            inputs[inputPad].values[i] = value;
        }
    }
    void block_setInput(int inputPad, const float* buffer) {
        memcpy(inputs[inputPad].values, buffer, sizeof(float)*Pad::k_blockSize);
    }
    void block_addToInput(int inputPad, const float* buffer) {
        for (int i = 0; i < Pad::k_blockSize; ++i) {
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
