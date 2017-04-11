#ifndef PHASE_HPP
#define PHASE_HPP

#include "module.hpp"

class Phase : public Module
{
public:
    Phase();
    void process(uint32_t fs);
    static Module* factory() { return new Phase(); }
};

#endif