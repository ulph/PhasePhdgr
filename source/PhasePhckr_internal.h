#pragma once

#include <math.h>
#include <vector>
#include <random>
#include <string.h>

#include "PhasePhckr.h"

namespace PhasePhckr {

    static float NoteToHz(float note) {
        return 440.f * powf(2.f, (note - 69.f) / 12.f);
    }

    const long double PI = 3.141592653589793238L;

    struct MPEVoiceState {
        float strikeZ; // velocity, 0 to 1
        float pressZ; // aftertouch, 0 to 1
        float liftZ; // release velocity, 0 to 1
        float slideY; // horizontal up/down, 0 to 1
        float glideX; // pitch bend, -1 to 1
        float pitchHz; // pitch in Hz, combination of root note and pitch bend with bend ranges taken into account
        float gate; // open or closed, for statey stuff
    };


    struct MPEVoiceConfig {
        float pitchRangeUp;
        float pitchRangeDown;
        // + any other config type of stuff like transpose, tuning etc.
        MPEVoiceConfig() :
            pitchRangeUp(24),
            pitchRangeDown(24)
        {}
    };


    class MPEVoice {
    public:
        MPEVoice();
        void on(int note, float velocity);
        void off(int note, float velocity);
        void glide(float glide);
        void slide(float slide);
        void press(float press);
        void update();
        void reset();
        const MPEVoiceState & getState();
    private:
        MPEVoiceState tg; // target, so we can slew x,y,z
        MPEVoiceState st; // state
        MPEVoiceConfig cfg;
        float slewFactor;
        int rootNote;
        void calculatePitchHz();
    };


    class SynthVoiceI {
    public:
        MPEVoice mpe;
        virtual void reset() = 0;        // TODO, map up all the rest as we 

        virtual void update(float * buffer, int numSamples, float sampleRate) = 0;
    };


    class ComponentI {
    public:
        virtual void reset(){}

        virtual void put(int port, float value){}
        virtual void update(){}
        virtual float get(int port){return 0;}

        int numberOfInputPorts();
        int numberOfOutputPorts();
        int getInputPortNumber(char * name);
        int getOutputPortNumber(char * name);
        void setSampleRate(float newSampleRate) {sampleRate = newSampleRate;}

    protected:
        float sampleRate;
        int findPortNumber(const std::vector<const char *> &portNames, const char * portName);
        std::vector<const char *> inputPortNames;
        std::vector<const char *> outputPortNames;
    };
}
