#pragma once

#include "module.hpp"

class Div : public ModuleCRTP<Div>
{
public:
    Div();
    void process(uint32_t fs);
    static Module* factory() { return new Div(); }
};

class Mod : public ModuleCRTP<Mod>
{
public:
    Mod();
    void process(uint32_t fs);
    static Module* factory() { return new Mod(); }
};
