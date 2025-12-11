#ifndef QUANTIZE_HPP
#define QUANTIZE_HPP

#include "module.hpp"

class Quantize : public ModuleCRTP<Quantize>
{
public:
    Quantize();
    void processSample(int sample) override;
    static Module* factory() { return new Quantize(); }
};

#endif
