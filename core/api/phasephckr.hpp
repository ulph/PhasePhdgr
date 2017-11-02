#pragma once

#include <math.h>
#include <vector>
#include <utility>
#include <random>
#include <string.h>
#include <atomic>
#include <map>

using namespace std;

namespace PhasePhckr {

    class SynthVoice;
    class EffectChain;
    class VoiceBus;
    class GlobalData;
    struct PatchDescriptor;
    class ComponentRegister;

    PatchDescriptor getExampleFxChain();
    PatchDescriptor getExampleVoiceChain();

}

#include "phasephckr/components.hpp"
#include "phasephckr/design.hpp"
#include "phasephckr/docs.hpp"
#include "phasephckr/scope.hpp"
#include "phasephckr/synth.hpp"
