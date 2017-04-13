#ifndef RLC_HPP
#define RLC_HPP

#include "module.hpp"

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

class OpenRcLp : public Module
{
public:
    OpenRcLp();
    void process(uint32_t fs);
    static Module* factory() { return new OpenRcLp(); }
};

class OpenRcHp : public Module
{
public:
    OpenRcHp();
    void process(uint32_t fs);
    static Module* factory() { return new OpenRcHp(); }
};

#endif
