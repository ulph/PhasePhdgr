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
    friend ConnectionGraph;

protected:
    std::string name;
    std::vector<Pad> inputs;
    std::vector<Pad> outputs;

public:
    virtual ~Module() {}
    virtual Module *clone() const = 0;
    void setInput(int inputPad, float value);
    virtual PhasePhckr::ModuleDoc makeDoc() const;
    virtual std::string docString() const;
    int getNumInputPads() const;
    int getNumOutputPads() const;
    int getInputPadFromName(std::string padName) const;
    int getOutputPadFromName(std::string padName) const;

private:
    virtual void process(uint32_t fs) = 0;
    float sample_getOutput(int outputPad) const;
    void sample_setInput(int inputPad, float value);
    void sample_resetInput(int inputPad);
    void sample_addToInput(int inputPad, float value);

    virtual void block_process(uint32_t fs);
    void block_getOutput(int outputPad, float* buffer) const;
    void block_setInput(int inputPad, float value);
    void block_setInput(int inputPad, const float* buffer);
    void block_resetInput(int inputPad);
    void block_addToInput(int inputPad, const float* buffer);

    void setName(const std::string &n);

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
