#ifndef RLC_HPP
#define RLC_HPP

#include "module.hpp"

static inline float CalcRcLp(float x1, float y0, float wc, float fsInv) {
    float d = 2.0f * (float)M_PI*wc * fsInv;
    float a = d / (d + 1.0f);
    return a*x1 + (1.0f - a)*y0;
}

static inline float CalcRcHp(float x1, float x0, float y0, float wc, float fsInv) {
    float a = 1.0f / ((2.0f * (float)M_PI*wc * fsInv) + 1.0f);
    return a*y0 + a*(x1 - x0);
}

class RcLp : public ModuleCRTP<RcLp>
{
public:
    RcLp();
    void process();
    static Module* factory() { return new RcLp(); }
};

class RcHp : public ModuleCRTP<RcHp>
{
private:
    float x;
public:
    RcHp();
    void process();
    static Module* factory() { return new RcHp(); }
};

class OpenRcLp : public ModuleCRTP<OpenRcLp>
{
public:
    OpenRcLp();
    void process();
    static Module* factory() { return new OpenRcLp(); }
};

class OpenRcHp : public ModuleCRTP<OpenRcHp>
{
public:
    OpenRcHp();
    void process();
    static Module* factory() { return new OpenRcHp(); }
};

class Lag : public ModuleCRTP<Lag>
{
public:
    Lag();
    void process();
    static Module* factory() { return new Lag(); }
};

#endif
