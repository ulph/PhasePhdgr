#ifndef CROSSFADE_HPP
#define CROSSFADE_HPP

#include "module.hpp"

class CrossFade : public ModuleCRTP<CrossFade>
{
public:
    CrossFade();
    void process();
    static Module* factory() { return new CrossFade(); }
};


#endif
