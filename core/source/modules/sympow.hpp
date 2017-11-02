#ifndef SYMPOW_HPP
#define SYMPOW_HPP

#include "module.hpp"

class SymPow : public ModuleCRTP<SymPow>
{
public:
    SymPow();
    void process(uint32_t fs);
    static Module* factory() { return new SymPow(); }
};


#endif
