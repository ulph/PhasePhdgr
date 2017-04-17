#ifndef DELAY_HPP
#define DELAY_HPP

#include "module.hpp"

const int c_delayBufferSize = 96000*5;

class Delay : public Module
{
private:
    float buffer[c_delayBufferSize];
    int readPosition;
public:
    Delay();
    void process(uint32_t fs);
    static Module* factory() { return new Delay(); }
};

#endif
