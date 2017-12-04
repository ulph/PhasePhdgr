#pragma once

#include "module.hpp"

class Threshold : public ModuleCRTP<Threshold>
{
public:
    Threshold();
    void process();
    static Module* factory() { return new Threshold(); }
};