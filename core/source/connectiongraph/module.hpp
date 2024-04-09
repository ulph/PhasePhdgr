#pragma once

#include <algorithm>
#include <string>
#include <vector>

class ModuleAccessor;

struct Pad {
   private:
    void init(float value) {
        for (int i = 0; i < k_blockSize; ++i)
            values[i] = value;
        std::transform(unit.begin(), unit.end(), unit.begin(), ::tolower);
        // TODO, sanitate name also
    }

   public:
    static const int k_blockSize = 64;
    std::string name = "";
    std::string unit = "";
    float values[Pad::k_blockSize] = {0.0f};
    Pad(const char* name) : name(name) { init(0.0f); }
    Pad(const char* name, float value) : name(name) { init(value); }
    Pad(const char* name, float value, const char* unit) : name(name), unit(unit) { init(value); }
    Pad(const char* name, const char* unit) : name(name), unit(unit) { init(0.0f); }
    void reset() {
        for (auto& v : values)
            v = 0.0f;
    }
};

class Module {
    friend class ModuleAccessor;

   protected:
    std::vector<Pad> inputs;
    std::vector<Pad> outputs;
    float fs = 48000.f;
    float fsInv = 1.f / fs;
    // TODO; std::map<int, float> designValues;

   public:
    virtual ~Module() {}
    virtual Module* clone() const = 0;
    virtual std::string docString() const { return "..."; }

   private:
    virtual void init() {};
    std::string name = "";
    virtual void processSample(int sample) = 0;
    virtual void processBlock() {
        // Default naive implementation.
        for (int sample = 0; sample < Pad::k_blockSize; ++sample) {
            processSample(sample);
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
        to share their design stages. + coupled with blockwise or similar calc of design values ->
    nice speedup!

        *tbd, but probably by connection alone and not value
        ** not so much sense in doing this scheme per sample anyway
    }
    */
};

template <class D>
class ModuleCRTP : public Module {
   public:
    virtual Module* clone() const { return new D(static_cast<D const&>(*this)); }
};
