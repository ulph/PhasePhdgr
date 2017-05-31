#ifndef QUANT8_HPP
#define QUANT8_HPP

#include "module.hpp"

class Quant8 : public ModuleCRTP<Quant8>
{
public:
    Quant8();
    void process(uint32_t fs);
    static Module* factory() { return new Quant8(); }
};

#endif
