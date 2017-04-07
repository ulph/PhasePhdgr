#ifndef CAMELENVELOPE_H_INCLUDED
#define CAMELENVELOPE_H_INCLUDED

#include "../PhasePhckr.h"

namespace PhasePhckr {
namespace Components {

class CamelEnvelope : public ComponentI { 
// two bumps
private:
    float onBumpHeight;
    float offBumpHeight;
    float onAttackSpeed;
    float onDecaySpeed;
    float offAttackSpeed;
    float offDecaySpeed;
    float sustainHeight;
    float gate;
    float gate_last;
    bool trigger;
    int samplesCtr;
    float value;
    float target_value;
    float slew;

public:
    CamelEnvelope();

    virtual void reset();
    virtual void update();
    virtual float get(int port);
    virtual void put(int port, float value);

    // TODO; turn into input ports instead
    void setSustainHeight(float level);
    void setOnBumpHeight(float level);
    void setOffBumpHeight(float level);
    void setOnAttackSpeed(float speed);
    void setOffAttackSpeed(float speed);
    void setOnDecaySpeed(float speed);
    void setOffDecaySpeed(float speed);
};

}
}

#endif  // CAMELENVELOPE_H_INCLUDED
