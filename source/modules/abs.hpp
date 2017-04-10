#ifndef ABS_HPP
#define ABS_HPP

#include "module.hpp"

class Abs : public Module
{
public:
    Abs();
    void process(uint32_t fs);
    static Module* factory() { return new Abs(); }
};

#endif
