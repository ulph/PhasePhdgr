#pragma once

#include "module.hpp"

class Threshold : public ModuleCRTP<Threshold>
{
public:
    Threshold();
    void process(uint32_t fs);
    static Module* factory() { return new Threshold(); }
};