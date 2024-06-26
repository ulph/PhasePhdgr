#ifndef SYMPOW_HPP
#define SYMPOW_HPP

#include "module.hpp"

class SymPow : public ModuleCRTP<SymPow>
{
public:
    SymPow();
    void process();
    static Module* factory() { return new SymPow(); }
};

class SymLog2 : public ModuleCRTP<SymLog2>
{
public:
    SymLog2();
    void process();
    static Module* factory() { return new SymLog2(); }
};

#endif
