#pragma once

#include <utility>
#include <vector>
#include <map>
#include <iostream>

#include "components.hpp"

class ConnectionGraph;
class ComponentRegister;

using namespace std;

namespace PhasePhckr {

/* Graph stuff */

struct ModuleVariable {
    string name;
    string type;
};

struct ModulePort {
    string module;
    string port;
};

struct ModulePortConnection {
    ModulePort source;
    ModulePort target;
};

struct ModulePortValue {
    ModulePort target;
    float value;
};

struct ConnectionGraphDescriptor {
    vector<ModuleVariable> modules;
    vector<ModulePortConnection> connections;
    vector<ModulePortValue> values;
};

/* Component (subgraph) stuff */

struct ModulePortAlias {
    string alias;
    vector<ModulePort> wrapped;
};

struct ComponentDescriptor {
    vector<PadDescription> inBus;
    vector<PadDescription> outBus;
    ConnectionGraphDescriptor graph;
    string docString;
};

/* Patch (supergraph) */

struct ModulePosition {
    int x;
    int y;
};

struct PatchDescriptor {
    ComponentDescriptor root;
    string docString;
    map<string, ComponentDescriptor> components;
    map<string, ModulePosition> layout;
};

/* Functions and aux types */

void designConnectionGraph(
    ConnectionGraph &connectionGraph,
    PatchDescriptor &description,
    map<string, int> &moduleHandles,
    const ComponentRegister & cp
);

const ModuleVariable c_EffectInput = {"inBus", "_EFFECTINPUT"};
const ModuleVariable c_EffectOutput = {"outBus", "_EFFECTOUTPUT"};
const ModuleVariable c_VoiceInput = {"inBus", "_VOICEINPUT" };
const ModuleVariable c_VoiceOutput = {"outBus", "_VOICEOUTPUT"};

}
