#ifndef NOISE_HPP
#define NOISE_HPP

#include "module.hpp"

class Noise : public Module
{
private:
    uint32_t val;
public:
    Noise();
    void process(uint32_t fs);
    static Module* factory() { return new Noise(); }
};

#endif
