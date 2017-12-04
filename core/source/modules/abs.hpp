#ifndef ABS_HPP
#define ABS_HPP

#include "module.hpp"

class Abs : public ModuleCRTP<Abs>
{
public:
    Abs();
    void process();
    void block_process();
    static Module* factory() { return new Abs(); }
};

#endif
