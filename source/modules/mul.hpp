#ifndef MUL_HPP
#define MUL_HPP

#include "module.hpp"

class Mul : public Module
{
public:
    Mul();
    void process(uint32_t fs);
    static Module* factory() { return new Mul(); }
};

class Gain : public Module
{
public:
    Gain();
    void process(uint32_t fs);
    static Module* factory() { return new Gain(); }
};

#endif
