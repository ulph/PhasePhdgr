#ifndef QUANTIZE_HPP
#define QUANTIZE_HPP

#include "module.hpp"

class Quantize : public ModuleCRTP<Quantize>
{
public:
    Quantize();
    void process();
    static Module* factory() { return new Quantize(); }
};

#endif
