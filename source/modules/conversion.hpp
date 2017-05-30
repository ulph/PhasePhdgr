#ifndef CONVERSION_HPP
#define CONVERSION_HPP

#include "module.hpp"

class TempoToTime : public Module
{
public:
    TempoToTime();
    void process(uint32_t fs);
    static Module* factory() { return new TempoToTime(); }
};


class Transpose : public Module
{
public:
    Transpose();
    void process(uint32_t fs);
    static Module* factory() { return new Transpose(); }
};


#endif
