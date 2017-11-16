#pragma once

#include "module.hpp"

class TanH : public ModuleCRTP<TanH>
{
public:
    TanH();
    void process(uint32_t fs);
    static Module* factory() { return new TanH(); }
};
