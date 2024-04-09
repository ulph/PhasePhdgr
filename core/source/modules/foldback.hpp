#ifndef FOLDBACK_HPP
#define FOLDBACK_HPP

#include "module.hpp"

class FoldBack : public ModuleCRTP<FoldBack>
{
protected:
    bool iterate(float *v, float scale, float th);
public:
    FoldBack();
    void processSample(int sample) override;
    static Module* factory() { return new FoldBack(); }
};

class Wrap : public ModuleCRTP<Wrap>
{
protected:
    bool iterate(float *v, float th);
public:
    Wrap();
    void processSample(int sample) override;
    static Module* factory() { return new Wrap(); }
};

#endif
