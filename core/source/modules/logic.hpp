#pragma once

#include "module.hpp"

class Threshold : public ModuleCRTP<Threshold>
{
public:
    Threshold();
    void process() override;
    static Module* factory() { return new Threshold(); }
};

class Counter : public ModuleCRTP<Counter>
{
private:
    float lastReset = 0.0f;
    float lastTrigger = 0.0f;
    float counter = 0.0f;
public:
    Counter();
    void process() override;
    static Module* factory() { return new Counter(); }
};
