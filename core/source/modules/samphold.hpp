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
    void processSample(int sample) override;
    static Module* factory() { return new SampleAndHold(); }
};

#endif
