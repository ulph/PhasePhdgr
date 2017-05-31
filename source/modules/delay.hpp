#ifndef DELAY_HPP
#define DELAY_HPP

#include "module.hpp"

class Delay : public ModuleCRTP<Delay>
{
private:
    float *buffer;
    int bufferSize;
    int readPosition;
public:
    Delay();
    virtual ~Delay();
    void process(uint32_t fs);
    static Module* factory() { return new Delay(); }
};

#endif
