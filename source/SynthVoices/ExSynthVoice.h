#ifndef EXSYNTHVOICE_H_INCLUDED
#define EXSYNTHVOICE_H_INCLUDED

#include "PhasePhckr.h"
#include "Components/CamelEnvelope.h"
#include "connectiongraph/connectiongraph.hpp"
#include "connectiongraph/module.hpp"

namespace PhasePhckr {

    // example stupid stuff to have _something_ working
    class ExSynthVoice : public SynthVoiceI {
      public:
        virtual void reset();
        virtual void update(float * buffer, int numSamples, float sampleRate);
      private:
        Components::CamelEnvelope env;
        Components::CamelEnvelope noiseEnv;
        long double angle;
    };


    // special modules for the bus
    class InputBusModule : public Module {
      // hooks up all the voice+global inputs and outputs
      public:
        InputBusModule() {
          outputs.push_back(Pad("Gate"));
          outputs.push_back(Pad("StrikeZ"));
          outputs.push_back(Pad("LiftZ"));
          outputs.push_back(Pad("PitchHz"));
          outputs.push_back(Pad("GlideX"));
          outputs.push_back(Pad("SlideY"));
          outputs.push_back(Pad("PressZ"));
        }

        void updateVoice(const MPEVoiceState &state) {
          outputs[0].value = state.gate;
          outputs[1].value = state.strikeZ;
          outputs[2].value = state.liftZ;
          outputs[3].value = state.pitchHz;
          outputs[4].value = state.glideX;
          outputs[5].value = state.slideY;
          outputs[6].value = state.pressZ;
        }

        virtual void process(uint32_t fs){};

    };


    class OutputBusModule : public Module {
      public:
        OutputBusModule(){
          inputs.push_back(Pad("MonoOut"));
          outputs.push_back(Pad("MonoOut"));
        }
        virtual void process(uint32_t fs){
          outputs[0].value = inputs[0].value;
        };
    };


    class ExConnectionGraphVoice : public SynthVoiceI {
      public:
        ExConnectionGraphVoice():connectionGraph(48000),t(0){

          inBusHandle = connectionGraph.addModule(new InputBusModule());
          int phase = connectionGraph.addModule("PHASE");
          int square = connectionGraph.addModule("SQUARE");
          int mul = connectionGraph.addModule("MUL");
          outBusHandle = connectionGraph.addModule(new OutputBusModule());

          connectionGraph.connect(inBusHandle, 3, phase);
          connectionGraph.connect(phase, square);
          connectionGraph.connect(square, mul, 0);
          connectionGraph.connect(inBusHandle, 6, mul, 0);
          connectionGraph.connect(mul, outBusHandle);
        }

        virtual void reset(){}

        virtual void update(float * buffer, int numSamples, float sampleRate){
          Module* bus = connectionGraph.getModule(inBusHandle);
          ((InputBusModule*)bus)->updateVoice(mpe.getState());
          for (int i = 0; i < numSamples; ++i) {
            connectionGraph.process(outBusHandle, t+i);
            buffer[i] += connectionGraph.getOutput(outBusHandle, 0);
          }
          t += numSamples;
        }

      private:
        ConnectionGraph connectionGraph;
        unsigned long t;
        int inBusHandle;
        int outBusHandle;
    };
    

}

#endif  // EXSYNTHVOICE_H_INCLUDED
