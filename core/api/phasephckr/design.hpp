#pragma once

#include <utility>
#include <vector>
#include <map>
#include <iostream>

class ConnectionGraph;

using namespace std;

namespace PhasePhckr {

class ComponentRegister;

/* Graph stuff */

struct ModuleVariable {
    string name;
    string type;
    bool operator ==(ModuleVariable const& other) {
        return other.name == name && other.type == type;
    }
};

struct ModulePort {
    string module;
    string port;
    bool operator ==(ModulePort const& other) {
        return other.module == module && other.port == port;
    }
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

struct PadDescription {
    string name;
    string unit;
    float defaultValue;
};

struct ComponentDescriptor {
    vector<PadDescription> inBus;
    vector<PadDescription> outBus;
    ConnectionGraphDescriptor graph;
    string docString;
};

/* Patch */

struct ModulePosition {
    int x;
    int y;
};

struct PatchDescriptor {
    ComponentDescriptor root;
    map<string, ComponentDescriptor> components;
    map<string, ModulePosition> layout;
};

/* Preset (voice patch + effect patch + parameters) */
// these are managed by JUCE layer

struct ParameterDescriptor {
    string id;
    int index;
    float value;
    float min;
    float max;
};

struct PresetDescriptor {
    PatchDescriptor voice;
    PatchDescriptor effect;
    vector<ParameterDescriptor> parameters;
};

/* Functions and aux types */

void designPatch(
    ConnectionGraph &connectionGraph,
    const PatchDescriptor &description,
    const vector<PadDescription>& inBus,
    const vector<PadDescription>& outBus,
    map<string, int> &moduleHandles,
    map<string, int> &parameterHandles,
    const ComponentRegister & cp
);

const char componentSeparator = '.';
const char parameterMarker = '=';
const char componentMarker = '@';

bool moduleNameIsValid(const string& moduleName);
bool componentNameIsValid(const string& componentName); // TODO componentType
// TODO parameterTypeIsValid
// TODO typeIsValid (checks that all uppercase?)

// TODO isParameter, isComponent (that checks first char but ignores the validity at large)

inline bool matches(const ModuleVariable& mv, const ModulePort& mp) {
    return mv.name == mp.module;
    // TODO make use of this
}


inline bool matches(const ModulePort& mp, const ModuleVariable& mv) {
    return matches(mv, mp);
}

extern const vector<PadDescription> c_effectChainInBus;
extern const vector<PadDescription> c_effectChainOutBus;

extern const vector<PadDescription> c_voiceChainInBus;
extern const vector<PadDescription> c_voiceChainOutBus;

extern const ModuleVariable c_inBus;
extern const ModuleVariable c_outBus;


}
