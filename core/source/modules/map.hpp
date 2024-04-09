#ifndef CLAMP_HPP
#define CLAMP_HPP

#include "module.hpp"

class Clamp : public ModuleCRTP<Clamp>
{
public:
    Clamp();
    void processSample(int sample);
    static Module* factory() { return new Clamp(); }
};

class RangeMap : public ModuleCRTP<RangeMap>
{
public:
    RangeMap();
    void processSample(int sample);
    static Module* factory() { return new RangeMap(); }
};

class ScaleShift : public ModuleCRTP<ScaleShift>
{
public:
    ScaleShift();
    void processSample(int sample);
    static Module* factory() { return new ScaleShift(); }
};

class ClampInv : public ModuleCRTP<ClampInv>
{
public:
    ClampInv();
    void processSample(int sample);
    static Module* factory() { return new ClampInv(); }
};

#endif
