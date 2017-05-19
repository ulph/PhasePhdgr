#ifndef CHAMBERLIN_HPP
#define CHAMBERLIN_HPP

#include "module.hpp"

class ChamberlinFilter : public Module
{
private:
    float band;
    float low;
    float high;
public:
    ChamberlinFilter();
    void process(uint32_t fs);
    static Module* factory() { return new ChamberlinFilter(); }
};


class OpenChamberlinFilter : public Module
{
public:
    OpenChamberlinFilter();
    void process(uint32_t fs);
    static Module* factory() { return new OpenChamberlinFilter(); }
};


#endif
