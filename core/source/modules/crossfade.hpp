#ifndef CROSSFADE_HPP
#define CROSSFADE_HPP

#include "module.hpp"

class CrossFade : public ModuleCRTP<CrossFade>
{
public:
    CrossFade();
    void process();
    static Module* factory() { return new CrossFade(); }
};

class FadeCross : public ModuleCRTP<FadeCross>
{
public:
    FadeCross();
    void process();
    static Module* factory() { return new FadeCross(); }
};

class Mux8 : public ModuleCRTP<Mux8>
{
public:
    Mux8();
    void process();
    static Module* factory() { return new Mux8(); }
};

class Mux4 : public ModuleCRTP<Mux4>
{
public:
    Mux4();
    void process();
    static Module* factory() { return new Mux4(); }
};

#endif
