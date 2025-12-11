#pragma once

#include "module.hpp"

class Div : public ModuleCRTP<Div>
{
public:
    Div();
    void processSample(int sample);
    static Module* factory() { return new Div(); }
};

class Mod : public ModuleCRTP<Mod>
{
public:
    Mod();
    void processSample(int sample);
    static Module* factory() { return new Mod(); }
};
