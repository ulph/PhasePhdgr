#ifndef SCALESHIFT_HPP
#define SCALESHIFT_HPP

#include "module.hpp"

class ScaleShift : public ModuleCRTP<ScaleShift>
{
public:
    ScaleShift();
    void process(uint32_t fs);
    static Module* factory() { return new ScaleShift(); }
};

#endif
