#ifndef CLAMP_HPP
#define CLAMP_HPP

#include "module.hpp"

class Clamp : public ModuleCRTP<Clamp>
{
public:
    Clamp();
    void process(uint32_t fs);
    static Module* factory() { return new Clamp(); }
};

#endif
