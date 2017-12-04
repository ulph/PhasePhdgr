#ifndef ABS_HPP
#define ABS_HPP

#include "module.hpp"

class Abs : public ModuleCRTP<Abs>
{
public:
    Abs();
    void process(uint32_t fs);
    void block_process(uint32_t fs);
    static Module* factory() { return new Abs(); }
};

#endif
