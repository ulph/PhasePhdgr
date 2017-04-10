#ifndef CAMELENVELOPE_HPP
#define CAMELENVELOPE_HPP

#include "module.hpp"

class CamelEnvelope : public Module
{
public:
    CamelEnvelope();
    void process(uint32_t fs);
    static Module* factory() { return new CamelEnvelope(); }

private:
    float gate;
    float target_value;
    float slew;
    int samplesCtr;
};

#endif
