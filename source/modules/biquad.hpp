#pragma once

#include "module.hpp"

class Biquad : public Module
{
public:
    Biquad();
    void process(uint32_t fs);
    static Module* factory() { return new Biquad(); }

private:
    float x1;
    float x2;
    float y1;
    float y2;
};
