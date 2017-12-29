#ifndef MUL_HPP
#define MUL_HPP

#include "module.hpp"

class Mul : public ModuleCRTP<Mul>
{
public:
    Mul();
    void process() override;
    void block_process() override;
    static Module* factory() { return new Mul(); }
};

class MulTri : public ModuleCRTP<MulTri>
{
public:
    MulTri();
    void process() override;
    void block_process() override;
    static Module* factory() { return new MulTri(); }
};

class MulQuad : public ModuleCRTP<MulQuad>
{
public:
    MulQuad();
    void process() override;
    void block_process() override;
    static Module* factory() { return new MulQuad(); }
};

class Gain : public ModuleCRTP<Gain>
{
public:
    Gain();
    void process() override;
    void block_process() override;
    static Module* factory() { return new Gain(); }
};

#endif
