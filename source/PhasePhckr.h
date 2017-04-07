#ifndef PHASEPHCKR_H_INCLUDED
#define PHASEPHCKR_H_INCLUDED

#include <math.h>
#include <vector>
#include <random>
#include <string.h>

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
        long samplesSinceGateOn; // for statey stuff (envelopes etc)
        long samplesSinceGateOff; // for statey stuff (release envelopes etc)
        bool gate; // open or closed, for statey stuff
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
        virtual void reset() = 0;
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
        void setSampleRate(float newSampleRate){sampleRate = newSampleRate;}

    protected:
        float sampleRate;
        int findPortNumber(const std::vector<const char *> &portNames, const char * portName);
        std::vector<const char *> inputPortNames;
        std::vector<const char *> outputPortNames;
    };


    class VoiceBus {
        private:
        std::vector<SynthVoiceI*> *voices;

        // TODO, make it work for normal polyphony as well
        // and use internal round-robin instead of relying of channel ...

        public:
        VoiceBus() : voices(nullptr){};
        VoiceBus(std::vector<SynthVoiceI*> * parent_voices) {
            voices = parent_voices;
        }

        void handleNoteOnOff(int channel, int note, float velocity, bool on){
            SynthVoiceI *v = (*voices)[channel];
            on ? v->mpe.on(note, velocity) : v->mpe.off(note, velocity);
        }

        void handleX(int channel, float position){
            (*voices)[channel]->mpe.glide(position);
        }

        void handleY(int channel, float position){
            (*voices)[channel]->mpe.slide(position);
        }

        void handleZ(int channel, float position){
            (*voices)[channel]->mpe.press(position);
        }

    };


    class AutomationBus {
        // handles the automation (global VST) parameters
    };

}

#endif  // PHASEPHCKR_H_INCLUDED
