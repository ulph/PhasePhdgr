#ifndef SINE_HPP
#define SINE_HPP

#include "module.hpp"

class Sine : public ModuleCRTP<Sine>
{
public:
    Sine();
    void processSample(int sample) override;
    void processBlock() override;
    static Module* factory() { return new Sine(); }
};

#endif
