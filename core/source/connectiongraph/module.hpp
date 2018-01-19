#pragma once

#include <vector>
#include <string>

class ModuleAccessor;

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
    friend class ModuleAccessor;

protected:
    std::vector<Pad> inputs;
    std::vector<Pad> outputs;
    float fs = 48000.f;
    float fsInv = 1.f / fs;
    // TODO; std::map<int, float> designValues;

public:
    virtual ~Module() {}
    virtual Module *clone() const = 0;
    virtual std::string docString() const { return "..."; }

private:
    virtual void init() {};
    std::string name = "";
    virtual void process() = 0;
    virtual void block_process() {
        // default naive implementation
        for (int i = 0; i < Pad::k_blockSize; ++i) {
            for (int k = 0; k < inputs.size(); ++k) inputs[k].value = inputs[k].values[i];
            process();
            for (int k = 0; k < outputs.size(); ++k) outputs[k].values[i] = outputs[k].value;
        }
    }

    // TODO;
    /*
    virtual void block_design(){
        use some inputs,
        calc some stuff
        store to designValues; key is input index, value is value after block_design

        then, in block_process**, use designValues instead of those previously used inputs

        this scheme allows identical modules with identical inputs* to designValues 
        to share their design stages. + coupled with blockwise or similar calc of design values -> nice speedup!

        *tbd, but probably by connection alone and not value
        ** not so much sense in doing this scheme per sample anyway
    }
    */
};

template <class D>
class ModuleCRTP : public Module {
public:
  virtual Module *clone() const {
      return new D(static_cast<D const&>(*this));
  }
};
