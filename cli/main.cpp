#include <stdio.h>

#include <functional>
#include <vector>
#include <utility>

#include <phasephckr.hpp>
#include <phasephckr_json.hpp>

#include <jack/jack.h>

using namespace PhasePhckr;

int main(int argc, char **argv)
{
  jack_options_t jo;
  jack_status_t js;
  jack_client_t* jc = jack_client_open("PhasePhckr", jo, &js);
  if(!jc) return -1;

  Synth synth;
  ComponentRegister comp;
  synth.setEffectChain(getExampleEffectChain(), comp);
  synth.setVoiceChain(getExampleVoiceChain(), comp);

  // setup

  if( auto r = jack_activate(jc) ) return r;

  // continously processing

  if( auto r = jack_deactivate(jc) ) return r;
  
  // cleanup

  return jack_client_close(jc);
}
