#pragma once

#include "module.hpp"

class TrapezoidalTanSVF :  public ModuleCRTP<TrapezoidalTanSVF>
{
    // cytomic.com/files/dsp/SvfLinearTrapOptimised2.pdf
    // TODO: cytomic.com/files/dsp/SvfLinearTrapezoidalSin.pdf instead
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
