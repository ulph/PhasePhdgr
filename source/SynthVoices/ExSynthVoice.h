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
        ExConnectionGraphVoice():
          connectionGraph(48000),
          t(0)
        {
          inBus = connectionGraph.addModule(new InputBusModule());
          outBus = connectionGraph.addModule(new OutputBusModule());

          int phase = connectionGraph.addModule("PHASE");
          int osc2 = connectionGraph.addModule("SINE");
          int osc3 = connectionGraph.addModule("SINE");
          int noise = connectionGraph.addModule("NOISE");
          int atan2 = connectionGraph.addModule("ATAN");
          int const2 = connectionGraph.addModule("CONST");
          int mul2 = connectionGraph.addModule("MUL"); 
          int mixEnv = connectionGraph.addModule("ENV");
          int mixAmp = connectionGraph.addModule("MUL");

          connectionGraph.connect(inBus, 3, phase, 0);
          connectionGraph.connect(inBus, 0, mixEnv, 0);
          connectionGraph.connect(inBus, 6, mixEnv, 4);

          connectionGraph.connect(phase, 0, osc3, 0);

          connectionGraph.connect(phase, 0, atan2, 0);
          connectionGraph.connect(const2, 0, mul2, 0);
          connectionGraph.connect(atan2, 0, mul2, 1);
          connectionGraph.connect(mul2, 0, osc2, 0);

//          connectionGraph.connect(osc2, 0, mixAmp, 0);
          connectionGraph.connect(osc3, 0, mixAmp, 0);
          connectionGraph.connect(mixEnv, 0, mixAmp, 1);

          connectionGraph.connect(mixAmp, 0, outBus, 0);
        }

        virtual void reset(){}

        virtual void update(float * buffer, int numSamples, float sampleRate){
          InputBusModule* inbusPtr = (InputBusModule*)connectionGraph.getModule(inBus);
          for (int i = 0; i < numSamples; ++i) {
            inbusPtr->updateVoice(mpe.getState());
            mpe.update();
            connectionGraph.process(outBus, t+i);
            buffer[i] += connectionGraph.getOutput(outBus, 0);
          }
          t += numSamples;
        }

      private:
        ConnectionGraph connectionGraph;
        unsigned long t;
        int inBus;
        int outBus;
    };


}

#endif  // EXSYNTHVOICE_H_INCLUDED
