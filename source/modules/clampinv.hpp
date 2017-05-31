#ifndef CLAMPINV_HPP
#define CLAMPINV_HPP

#include "module.hpp"

class ClampInv : public ModuleCRTP<ClampInv>
{
public:
    ClampInv();
    void process(uint32_t fs);
    static Module* factory() { return new ClampInv(); }
};


#endif
