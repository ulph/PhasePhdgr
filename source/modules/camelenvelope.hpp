#ifndef CAMELENVELOPE_HPP
#define CAMELENVELOPE_HPP

#include "module.hpp"

class CamelEnvelope : public ModuleCRTP<CamelEnvelope>
{
public:
    CamelEnvelope();
    void process(uint32_t fs);
    static Module* factory() { return new CamelEnvelope(); }
    virtual std::string docString() { return "Trigger attack/release stages on gate flank. Can behave like a traditional ADSR, but also has a 'release' bump. The shapes are controllable from convex, through linear to concave via a power law."; };
private:
    float gate;
    float gateOnTargetValue;
    float slew;
    int samplesCtr;
};

#endif
