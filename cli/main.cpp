#include <unistd.h>
#include <signal.h>

#include <stdio.h>

#include <functional>
#include <vector>
#include <utility>

#include <phasephckr.hpp>
#include <phasephckr_json.hpp>

#include <jack/jack.h>
#include <jack/midiport.h>

#include <iostream>

using namespace PhasePhckr;

class PhasePhckrJackApp {
  public:
    Synth synth;
    ComponentRegister comp;

    jack_port_t *in_midi;

	  jack_port_t *out_left;
    jack_port_t *out_right;

    jack_default_audio_sample_t fs;

    jack_options_t jo = JackNullOption;
    jack_status_t js;
    jack_client_t* jc = nullptr;

    void handle_midi(const jack_midi_event_t& in_event){
      // TODO
    }

    int process(jack_nframes_t nframes){
      const int sz = Synth::internalBlockSize();

      if(nframes % sz != 0) return -1;
     
	    jack_midi_event_t in_event;
      jack_nframes_t frame_index = 0;
	    jack_nframes_t event_index = 0;

      void* port_buf = jack_port_get_buffer(in_midi, nframes);
	    jack_nframes_t event_count = jack_midi_get_event_count(port_buf);

      auto *l = (jack_default_audio_sample_t *) jack_port_get_buffer(out_left, nframes);
      auto *r = (jack_default_audio_sample_t *) jack_port_get_buffer(out_right, nframes);

      while(frame_index < nframes) {
        while(event_index < event_count){
          jack_midi_event_get(&in_event, port_buf, event_index);
          if(in_event.time < frame_index + sz){
            handle_midi(in_event);
            event_index++;
          }
          else break;
        }
        synth.update(&l[frame_index], &r[frame_index], sz, fs);
        frame_index += sz;
      }

      return 0;
    }

  private:
    simple_lock synth_lock;
};


int sample_rate_changed(jack_nframes_t nframes, void *arg) {
  auto app = static_cast<PhasePhckrJackApp *>(arg);
	app->fs = (jack_default_audio_sample_t)nframes;
	return 0;
}


int process (jack_nframes_t nframes, void *arg) {
  auto app = static_cast<PhasePhckrJackApp *>(arg);
  return app->process(nframes);
}


static void signal_handler(int sig) {
	exit(0);
}


static void shutdown(void *arg) {
  exit(1);
}


int main(int argc, char **argv) {
  // setup PhasePhckr
  PhasePhckrJackApp app;

  app.jc = jack_client_open("PhasePhckr", app.jo, &app.js);
  if(!app.jc) return -1;

  app.synth.setEffectChain(getExampleEffectChain(), app.comp);
  app.synth.setVoiceChain(getExampleVoiceChain(), app.comp);

  // setup jack
  jack_set_process_callback(app.jc, process, &app);
  app.in_midi = jack_port_register(app.jc, "midi_in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
	app.out_left = jack_port_register(app.jc, "audio_out_left", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	app.out_right = jack_port_register(app.jc, "audio_out_right", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
  if((app.in_midi == NULL) || (app.out_left == NULL) || (app.out_right == NULL)) return -1;
  app.fs = jack_get_sample_rate(app.jc);

  // handle signals etc
  jack_on_shutdown (app.jc, shutdown, 0);
  signal(SIGQUIT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGHUP, signal_handler);
  signal(SIGINT, signal_handler);

  // activate 
  if(auto r = jack_activate(app.jc)) return r;

  #if 0
  // connect to hw output
  auto ports = jack_get_ports(app.jc, NULL, NULL, JackPortIsPhysical|JackPortIsInput);
  if(ports == NULL) return -1;
  if(auto r = jack_connect(app.jc, jack_port_name(app.out_left), ports[0])) return r;
  if(auto r = jack_connect(app.jc, jack_port_name(app.out_right), ports[1])) return r;
  jack_free(ports);
  #endif

  // process spin
  while(1){ sleep(1); }
  
  // shutdown jack
  if( auto r = jack_deactivate(app.jc) ) return r;
  return jack_client_close(app.jc);
}
