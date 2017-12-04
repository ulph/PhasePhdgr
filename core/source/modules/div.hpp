#pragma once

#include "module.hpp"

class Div : public ModuleCRTP<Div>
{
public:
    Div();
    void process();
    static Module* factory() { return new Div(); }
};

class Mod : public ModuleCRTP<Mod>
{
public:
    Mod();
    void process();
    static Module* factory() { return new Mod(); }
};
