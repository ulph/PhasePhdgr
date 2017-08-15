#ifndef SAMPHOLD_HPP
#define SAMPHOLD_HPP

#include "module.hpp"

class SampleAndHold : public ModuleCRTP<SampleAndHold>
{
private:
    float lastTrigger;
    float heldValue;
public:
    SampleAndHold();
    void process(uint32_t fs);
    static Module* factory() { return new SampleAndHold(); }
};

#endif
