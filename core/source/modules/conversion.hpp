#ifndef CONVERSION_HPP
#define CONVERSION_HPP

#include "module.hpp"

class TempoToTime : public ModuleCRTP<TempoToTime>
{
public:
    TempoToTime();
    void processSample(int sample) override;
    static Module* factory() { return new TempoToTime(); }
};


class Transpose : public ModuleCRTP<Transpose>
{
public:
    Transpose();
    void processSample(int sample) override;
    static Module* factory() { return new Transpose(); }
};


#endif
