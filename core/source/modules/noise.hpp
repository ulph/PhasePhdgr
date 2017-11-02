#ifndef NOISE_HPP
#define NOISE_HPP

#include "module.hpp"

class Noise : public ModuleCRTP<Noise>
{
private:
    uint32_t val;
public:
    Noise();
    void process(uint32_t fs);
    static Module* factory() { return new Noise(); }
    virtual Module *clone() const; // to initialize seed
};

#endif
