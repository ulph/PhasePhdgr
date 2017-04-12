#ifndef SATURATORATAN_HPP
#define SATURATORATAN_HPP

#include "module.hpp"

class SaturatorAtan : public Module
{
public:
    SaturatorAtan();
    void process(uint32_t fs);
    static Module* factory() { return new SaturatorAtan(); }
};

class StereoSaturatorAtan : public Module
{
public:
    StereoSaturatorAtan();
    void process(uint32_t fs);
    static Module* factory() { return new StereoSaturatorAtan(); }
};

#endif
