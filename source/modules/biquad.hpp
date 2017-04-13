#pragma once

#include "module.hpp"

class Biquad : public Module
{
public:
    Biquad();
    virtual void process(uint32_t fs);
    static Module* factory() { return new Biquad(); }

private:
    void init_state();

    float x1;
    float x2;
    float y1;
    float y2;
};

class LowPass : public Module
{
public:
    LowPass();
    virtual void process(uint32_t fs);
    static Module* factory() { return new LowPass(); }
};
