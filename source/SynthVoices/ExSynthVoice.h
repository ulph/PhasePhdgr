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
          // approximation of the patch in ExSynthVoice. no noise, less envelope tweaks etc.
          //
          // we lack a _lot_ of nuances as it's a royal pain to scale, add bias, invert etc
          // need to figure out a good way to streamline that as number of modules and 
          // connections grows painfully fast
          //
          // also, ports summation on input is really needed, 
          // or number of modules needed explodes for anything but the most trivial

          inBus = connectionGraph.addModule(new InputBusModule());
          outBus = connectionGraph.addModule(new OutputBusModule());

          int phase = connectionGraph.addModule("PHASE");
          int mul = connectionGraph.addModule("MUL"); 
          int mixEnv = connectionGraph.addModule("ENV");

          int atan = connectionGraph.addModule("ATAN");
          int atanMul = connectionGraph.addModule("MUL");
          int osc1 = connectionGraph.addModule("SINE");
          int osc2 = connectionGraph.addModule("SINE");
          int osc3mul = connectionGraph.addModule("MUL");
          int osc3 = connectionGraph.addModule("SINE");
          int osc23 = connectionGraph.addModule("XFADE");

          connectionGraph.connect(inBus, 0, mixEnv, 0);
          connectionGraph.connect(inBus, 1, mixEnv, 1);
          connectionGraph.connect(inBus, 2, mixEnv, 5);
          connectionGraph.connect(inBus, 6, mixEnv, 4);
          connectionGraph.connect(inBus, 3, phase, 0);

          // osc 1, f0 cohesion...
          connectionGraph.connect(phase, 0, osc1, 0);

          // osc 2 and osc 3, crossfade on Y and atan prescale on Z (... env)
          connectionGraph.connect(phase, 0, atan, 0);
          connectionGraph.connect(mixEnv, 0, atanMul, 0);
          connectionGraph.getModule(atanMul)->setInput(1, 10); // cheating!
          connectionGraph.connect(atanMul, 0, atan, 1);
          connectionGraph.connect(atan, 0, osc2, 0);

          connectionGraph.connect(atan, 0, osc3mul, 0);
          connectionGraph.getModule(osc3mul)->setInput(1, 2); // cheating, again!
          connectionGraph.connect(osc3mul, 0, osc3, 0);

          connectionGraph.connect(osc2, 0, osc23, 0);
          connectionGraph.connect(osc3, 0, osc23, 1);
          connectionGraph.connect(inBus, 5, osc23, 2);

          // mix amplitude on Z (... env)
          int mix = connectionGraph.addModule("ADD");
          connectionGraph.connect(osc1, 0, mix, 0);
          connectionGraph.connect(osc23, 0, mix, 1);
          connectionGraph.connect(mix, 0, mul, 0);
          connectionGraph.connect(mixEnv, 0, mul, 1);

          connectionGraph.connect(mul, 0, outBus, 0);
        }

        virtual void reset(){}

        virtual void update(float * buffer, int numSamples, float sampleRate){
          InputBusModule* inbusPtr = (InputBusModule*)connectionGraph.getModule(inBus);
          for (int i = 0; i < numSamples; ++i) {
            inbusPtr->updateVoice(mpe.getState());
            mpe.update();
            connectionGraph.process(outBus, t+i);
            buffer[i] += 0.5*connectionGraph.getOutput(outBus, 0);
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
