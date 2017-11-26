#ifndef SCALESHIFT_HPP
#define SCALESHIFT_HPP

#include "module.hpp"

class ScaleShift : public ModuleCRTP<ScaleShift>
{
public:
    ScaleShift();
    void process(uint32_t fs);
    void block_process(uint32_t fs) override;
    virtual std::string docString() {
        return "a * b + c";
    }
    static Module* factory() { return new ScaleShift(); }
};

class ShiftScale : public ModuleCRTP<ShiftScale>
{
public:
    ShiftScale();
    void process(uint32_t fs);
    void block_process(uint32_t fs) override;
    virtual std::string docString() {
        return "(a + b) * c";
    }
    static Module* factory() { return new ShiftScale(); }
};

class ScaleShiftMul : public ModuleCRTP<ScaleShiftMul>
{
public:
    ScaleShiftMul();
    void process(uint32_t fs);
    void block_process(uint32_t fs) override;
    virtual std::string docString() {
        return "(a * b + c) * d";
    }
    static Module* factory() { return new ScaleShiftMul(); }
};

#endif
