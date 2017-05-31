#ifndef CONVERSION_HPP
#define CONVERSION_HPP

#include "module.hpp"

class TempoToTime : public ModuleCRTP<TempoToTime>
{
public:
    TempoToTime();
    void process(uint32_t fs);
    static Module* factory() { return new TempoToTime(); }
};


class Transpose : public ModuleCRTP<Transpose>
{
public:
    Transpose();
    void process(uint32_t fs);
    static Module* factory() { return new Transpose(); }
};


#endif
