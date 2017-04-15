#ifndef DELAY_HPP
#define DELAY_HPP

#include "module.hpp"

class Delay : public Module
{
private:
    float buffer[96000*5];
    int readPosition;
public:
    Delay();
    void process(uint32_t fs);
    static Module* factory() { return new Delay(); }
};

#endif
