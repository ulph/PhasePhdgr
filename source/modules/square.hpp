#ifndef SQUARE_HPP
#define SQUARE_HPP

#include "module.hpp"

class Square : public Module
{
public:
    Square();
    void process(uint32_t fs);
    static Module* factory() { return new Square(); }
};

#endif
