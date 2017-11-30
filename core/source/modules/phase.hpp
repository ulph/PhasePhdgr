#ifndef PHASE_HPP
#define PHASE_HPP

#include "module.hpp"

class Phase : public ModuleCRTP<Phase>
{
private:
    float trig;
    float phase;
public:
    Phase();
    void process(uint32_t fs);
    void block_process(uint32_t fs);
    static Module* factory() { return new Phase(); }
};

#endif
