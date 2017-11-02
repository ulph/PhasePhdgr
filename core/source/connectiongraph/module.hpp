#ifndef MODULE_HPP
#define MODULE_HPP

#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <vector>
#include <string>
#include "docs.hpp"

struct Pad
{
    std::string name;
    float value;
    std::string unit;
    Pad(const char *name) : name(name), value(0.0f), unit("") {}
    Pad(const char *name, float value) : name(name), value(value), unit("") {}
    Pad(const char *name, float value, const char *unit) : name(name), value(value), unit(unit) {}
    Pad(const char *name, const char *unit) : name(name), value(0.0f), unit(unit) {}
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
    float getOutput(int outputPad) { 
        return outputs[outputPad].value;
    }
    void setInput(int inputPad, float value) {
        inputs[inputPad].value = value;
    }
    void addToInput(int inputPad, float value) {
        inputs[inputPad].value += value;
    }
    int getNumInputPads() { return (int)inputs.size(); }
    int getNumOutputPads() { return (int)outputs.size(); }
    
    int getInputPadFromName(std::string padName) {
        for(int i = 0; i < (int)inputs.size(); i++) {
            if(inputs[i].name == padName) {
                return i;
            }
        }
        // Not found
        std::cerr << "Error: Cannot find input pad " << padName << " (" << name << ")" << std::endl;
        return -1;
    }
    
    int getOutputPadFromName(std::string padName) {
        for(int i = 0; i < (int)outputs.size(); i++) {
            if(outputs[i].name == padName) {
                return i;
            }
        }
        // Not found
        std::cerr << "Error: Cannot find output pad " << padName << " (" << name << ")" << std::endl;
        return -1;
    }

    void setName(const std::string &n) { name = n; }
    std::string getName() { return name; }

    virtual std::string docString() { return "..."; }
    virtual ModuleDoc makeDoc() {
        ModuleDoc doc;
        doc.type = name;
        doc.docString = docString();
        for (const auto p : inputs) {
            PadDescription pd;
            pd.name = p.name;
            pd.unit = p.unit;
            pd.value = p.value;
            doc.inputs.push_back(pd);
        }
        for (const auto p : outputs) {
            PadDescription pd;
            pd.name = p.name;
            pd.unit = p.unit;
            pd.value = p.value;
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
