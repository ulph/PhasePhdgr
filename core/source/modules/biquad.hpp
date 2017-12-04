#pragma once

#include "module.hpp"

class Biquad : public ModuleCRTP<Biquad>
{
public:
    Biquad();
    virtual void process();
    static Module* factory() { return new Biquad(); }

private:
    void init_state();

    float x1;
    float x2;
    float y1;
    float y2;
};

class LowPass : public ModuleCRTP<LowPass>
{
public:
    LowPass();
    virtual void process();
    static Module* factory() { return new LowPass(); }
};

class PeakingEQ : public ModuleCRTP<PeakingEQ>
{
public:
  PeakingEQ();
  virtual void process();
  static Module *factory() {return new PeakingEQ(); }
};
