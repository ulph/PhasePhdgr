#ifndef NOISE_HPP
#define NOISE_HPP

#include <cstdint>
#include "module.hpp"

class Noise : public ModuleCRTP<Noise>
{
private:
    uint32_t val;
public:
    Noise();
    void process();
    static Module* factory() { return new Noise(); }
};

#endif
