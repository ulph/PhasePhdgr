#ifndef RLC_HPP
#define RLC_HPP

#include "module.hpp"

static inline float CalcRcLp(float x1, float y0, float wc, float fs) {
    float d = 2.0f * (float)M_PI*wc / (float)fs;
    float a = d / (d + 1.0f);
    return a*x1 + (1.0f - a)*y0;
}

static inline float CalcRcHp(float x1, float x0, float y0, float wc, float fs) {
    float a = 1.0f / ((2.0f * (float)M_PI*wc / (float)fs) + 1.0f);
    return a*y0 + a*(x1 - x0);
}

class RcLp : public ModuleCRTP<RcLp>
{
public:
    RcLp();
    void process(uint32_t fs);
    static Module* factory() { return new RcLp(); }
};

class RcHp : public ModuleCRTP<RcHp>
{
private:
    float x;
public:
    RcHp();
    void process(uint32_t fs);
    static Module* factory() { return new RcHp(); }
};

class OpenRcLp : public ModuleCRTP<OpenRcLp>
{
public:
    OpenRcLp();
    void process(uint32_t fs);
    static Module* factory() { return new OpenRcLp(); }
};

class OpenRcHp : public ModuleCRTP<OpenRcHp>
{
public:
    OpenRcHp();
    void process(uint32_t fs);
    static Module* factory() { return new OpenRcHp(); }
};

class Lag : public ModuleCRTP<Lag>
{
public:
    Lag();
    void process(uint32_t fs);
    static Module* factory() { return new Lag(); }
};

#endif
