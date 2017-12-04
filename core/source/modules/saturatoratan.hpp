#ifndef SATURATORATAN_HPP
#define SATURATORATAN_HPP

#include "module.hpp"

class Atan : public ModuleCRTP<Atan>
{
public:
    Atan();
    void process();
    void block_process();
    static Module* factory() { return new Atan(); }
};

class SaturatorAtan : public ModuleCRTP<SaturatorAtan>
{
public:
    SaturatorAtan();
    void process();
    void block_process();
    static Module* factory() { return new SaturatorAtan(); }
};

class StereoSaturatorAtan : public ModuleCRTP<StereoSaturatorAtan>
{
public:
    StereoSaturatorAtan();
    void process();
    void block_process();
    static Module* factory() { return new StereoSaturatorAtan(); }
};

#endif
