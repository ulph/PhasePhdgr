#ifndef RLC_HPP
#define RLC_HPP

#include <assert.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "module.hpp"

static inline float DesignRcLp(float wc, float fsInv) {
    float d = 2.0f * (float)M_PI*wc*fsInv;
    assert(wc >= 0.0f);
    assert(fsInv >= 0.0f);
    assert(d + 1.0f > 0.0f);
    float a = d / (d + 1.0f);
    assert(a >= 0.0f);
    assert(a <= 1.0f);
    return a;
}

static inline float CalcRcLp(float x1, float y0, float wc, float fsInv) {
    float a = DesignRcLp(wc, fsInv);
    return a*x1 + (1.0f - a)*y0;
}

static inline float CalcRcHp(float x1, float x0, float y0, float wc, float fsInv) {
    float a = 1.0f / ((2.0f * (float)M_PI*wc * fsInv) + 1.0f);
    assert(a >= 0.0f);
    assert(a <= 1.0f);
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

class Lag : public ModuleCRTP<Lag>
{
public:
    Lag();
    void process();
    static Module* factory() { return new Lag(); }
};

class LeakyIntegrator : public ModuleCRTP<LeakyIntegrator>
{
private:
    float cumSum = 0.0f;
    float last_cumSum = 0.0f;
    float last_output = 0.0f;
public:
    LeakyIntegrator();
    void process();
    static Module* factory() { return new LeakyIntegrator(); }
};

class RateLimiter : public ModuleCRTP<RateLimiter>
{
public:
    RateLimiter();
    void process();
    static Module* factory() { return new RateLimiter(); }
};

#endif
