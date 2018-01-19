#pragma once

#include "module.hpp"

// from cytomic.com/files/dsp

class TrapezoidalTanSVF :  public ModuleCRTP<TrapezoidalTanSVF>
{
    // SvfLinearTrapOptimised2.pdf
    // TODO: SvfLinearTrapezoidalSin.pdf instead
protected:
    float ic1eq = 0.0f;
    float ic2eq = 0.0f;
public:
    TrapezoidalTanSVF();
    virtual void process();
    static Module* factory() { return new TrapezoidalTanSVF(); }
};

class OpenTrapezoidalTanSVF : public TrapezoidalTanSVF
{
private:
    size_t inPadOffset = 0;
public:
    OpenTrapezoidalTanSVF();
    virtual void process();
    static Module* factory() { return new OpenTrapezoidalTanSVF(); }
    virtual Module *clone() const { return new OpenTrapezoidalTanSVF(*this); }
};

class OpenTrapezoidalTanSVF2 : public ModuleCRTP<OpenTrapezoidalTanSVF2>
{
private:
    size_t inPadOffset = 0;
    size_t outPadOffset = 0;
public:
    OpenTrapezoidalTanSVF2();
    virtual void process();
    static Module* factory() { return new OpenTrapezoidalTanSVF2(); }
};

/*
// OnePoleLinearLowPass.pdf

passive:
y[n] = (g x[n] + ic1eq[n-1] )ê / (1 + g)
ic1eq[n] = 2 y[n] - ic1eq[n-1]

vs

active:
y[n] = g (x[n] - y[n-1] ) + ic1eq[n-1]
ic1eq[n] = 2 y[n] - ic1eq[n-1]

?

*/
