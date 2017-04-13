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

class RcLp : public Module
{
public:
    RcLp();
    void process(uint32_t fs);
    static Module* factory() { return new RcLp(); }
};

class RcHp : public Module
{
private:
    float x;
public:
    RcHp();
    void process(uint32_t fs);
    static Module* factory() { return new RcHp(); }
};

#endif
