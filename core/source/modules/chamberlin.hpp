#ifndef CHAMBERLIN_HPP
#define CHAMBERLIN_HPP

#include "module.hpp"

class ChamberlinFilter : public ModuleCRTP<ChamberlinFilter>
{
private:
    float band;
    float low;
    float high;
public:
    ChamberlinFilter();
    void process();
    static Module* factory() { return new ChamberlinFilter(); }
};


class OpenChamberlinFilter : public ModuleCRTP<OpenChamberlinFilter>
{
public:
    OpenChamberlinFilter();
    void process();
    static Module* factory() { return new OpenChamberlinFilter(); }
};


#endif
