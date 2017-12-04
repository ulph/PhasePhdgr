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
    void process();
    void block_process();
    static Module* factory() { return new Phase(); }
};

#endif
