#pragma once

#include "module.hpp"

class Zdf1p : public ModuleCRTP<Zdf1p> {
    float z1 = 0.0f;
public:
    Zdf1p();
    virtual void process() override;
    static Module* factory() { return new Zdf1p(); }
};

class Zdf1pLowShelf : public ModuleCRTP<Zdf1pLowShelf> {
    float z1l = 0.0f;
public:
    Zdf1pLowShelf();
    virtual void process() override;
    static Module* factory() { return new Zdf1pLowShelf(); }
};

class Zdf1pHighShelf : public ModuleCRTP<Zdf1pHighShelf> {
    float z1h = 0.0f;
public:
    Zdf1pHighShelf();
    virtual void process() override;
    static Module* factory() { return new Zdf1pHighShelf(); }
};