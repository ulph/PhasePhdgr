#ifndef LAG_HPP
#define LAG_HPP

#include "module.hpp"

class Lag : public Module
{
public:
    Lag();
    void process(uint32_t fs);
    static Module* factory() { return new Lag(); }
};


#endif
