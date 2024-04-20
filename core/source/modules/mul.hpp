#ifndef MUL_HPP
#define MUL_HPP

#include "module.hpp"

class Mul : public ModuleCRTP<Mul>
{
public:
    Mul();
    void processSample(int sample) override;
    void processBlock() override;
    static Module* factory() { return new Mul(); }
};

class MulTri : public ModuleCRTP<MulTri>
{
public:
    MulTri();
    void processSample(int sample) override;
    void processBlock() override;
    static Module* factory() { return new MulTri(); }
};

class MulQuad : public ModuleCRTP<MulQuad>
{
public:
    MulQuad();
    void processSample(int sample) override;
    void processBlock() override;
    static Module* factory() { return new MulQuad(); }
};

class Gain : public ModuleCRTP<Gain>
{
public:
    Gain();
    void processSample(int sample) override;
    void processBlock() override;
    static Module* factory() { return new Gain(); }
};

#endif
