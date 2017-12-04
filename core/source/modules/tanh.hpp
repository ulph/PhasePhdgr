#pragma once

#include "module.hpp"

class TanH : public ModuleCRTP<TanH>
{
public:
    TanH();
    void process();
    static Module* factory() { return new TanH(); }
};

class NormalizedTanH : public ModuleCRTP<NormalizedTanH>
{
public:
    NormalizedTanH();
    void process();
    static Module* factory() { return new NormalizedTanH(); }
};