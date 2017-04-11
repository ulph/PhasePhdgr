#ifndef SINE_HPP
#define SINE_HPP

#include "module.hpp"

class Sine : public Module
{
public:
    Sine();
    void process(uint32_t fs);
    static Module* factory() { return new Sine(); }
};

#endif