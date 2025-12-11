#pragma once

#include "module.hpp"

class Zdf1p : public ModuleCRTP<Zdf1p> {
    float s = 0.0f;
public:
    Zdf1p();
    virtual void processSample(int sample) override;
    static Module* factory() { return new Zdf1p(); }
};

class Zdf1pLowShelf : public ModuleCRTP<Zdf1pLowShelf> {
    float s = 0.0f;
public:
    Zdf1pLowShelf();
    virtual void processSample(int sample) override;
    static Module* factory() { return new Zdf1pLowShelf(); }
};

class Zdf1pHighShelf : public ModuleCRTP<Zdf1pHighShelf> {
    float s = 0.0f;
public:
    Zdf1pHighShelf();
    virtual void processSample(int sample) override;
    static Module* factory() { return new Zdf1pHighShelf(); }
};

class Zdf4pLadder : public ModuleCRTP<Zdf4pLadder> {
    float s1 = 0.0f;
    float s2 = 0.0f;
    float s3 = 0.0f;
    float s4 = 0.0f;
    float S5 = 0.0f;
public:
    Zdf4pLadder();
    virtual void processSample(int sample) override;
    static Module* factory() { return new Zdf4pLadder(); }
};
