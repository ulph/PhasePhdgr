#ifndef FOLDBACK_HPP
#define FOLDBACK_HPP

#include "module.hpp"

class FoldBack : public ModuleCRTP<FoldBack>
{
protected:
    bool iterate(float *v, float scale);
public:
    FoldBack();
    void process();
    static Module* factory() { return new FoldBack(); }
};

#endif
