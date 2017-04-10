#ifndef QUANT8_HPP
#define QUANT8_HPP

#include "module.hpp"

class Quant8 : public Module
{
public:
    Quant8();
    void process(uint32_t fs);
    static Module* factory() { return new Quant8(); }
};

#endif
