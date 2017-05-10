#pragma once

#include <utility>
#include <vector>
#include <map>
#include <iostream>

#include "components.hpp"
#include "docs.hpp"

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
    map<string, ComponentDescriptor> components;
    map<string, ModulePosition> layout;
};

/* Functions and aux types */

void designPatch(
    ConnectionGraph &connectionGraph,
    PatchDescriptor &description,
    const vector<PadDescription>& inBus,
    const vector<PadDescription>& outBus,
    map<string, int> &moduleHandles,
    const ComponentRegister & cp
);

const vector<PadDescription> c_effectChainInBus = {
    {"left", "", 0},
    {"right", "", 0},
    {"mod", "", 0},
    {"exp", "", 0},
    {"brt", "", 0}
};

const vector<PadDescription> c_effectChainOutBus = {
    {"left", "", 0},
    {"right", "", 0},
};

const vector<PadDescription> c_voiceChainInBus = {
    {"gate", "", 0},
    {"strike_z", "", 0},
    {"lift_z", "", 0},
    {"pitch_hz", "", 0},
    {"glide_x", "", 0},
    {"slide_y", "", 0},
    {"press_z", "", 0},
    {"mod", "", 0},
    {"exp", "", 0},
    {"brt", "", 0}
};

const vector<PadDescription> c_voiceChainOutBus = {
    {"left", "", 0},
    {"right", "", 0},
};

}
