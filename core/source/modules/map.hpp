#ifndef CLAMP_HPP
#define CLAMP_HPP

#include "module.hpp"

class Clamp : public ModuleCRTP<Clamp>
{
public:
    Clamp();
    void process();
    static Module* factory() { return new Clamp(); }
};

class RangeMap : public ModuleCRTP<RangeMap>
{
public:
    RangeMap();
    void process();
    static Module* factory() { return new RangeMap(); }
};

#endif
