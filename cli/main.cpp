#include <stdio.h>

#include <functional>
#include <vector>
#include <utility>

#include <phasephckr.hpp>
#include <phasephckr_json.hpp>


using namespace PhasePhckr;


int main(int argc, char **argv)
{
  Synth synth;
  ComponentRegister comp;
  synth.setFxChain(getExampleFxChain(), comp);
  synth.setVoiceChain(getExampleVoiceChain(), comp);
}
