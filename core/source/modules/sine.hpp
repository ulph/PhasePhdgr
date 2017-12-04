#ifndef SINE_HPP
#define SINE_HPP

#include "module.hpp"

class Sine : public ModuleCRTP<Sine>
{
public:
    Sine();
    void process();
    void block_process();
    static Module* factory() { return new Sine(); }
};

#endif
