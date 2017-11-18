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
    explicit operator pair<string, string>() const { return make_pair(module, port); }
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

enum SynthGraphType {
    UNDEFINED = 0,
    VOICE = 1,
    EFFECT = 2
};

struct PatchParameterDescriptor {
    SynthGraphType type;
    string id;
    float val;
    float min;
    float max;
};

typedef map<int, PatchParameterDescriptor> ParameterHandleMap;

struct ComponentDescriptor {
    vector<PadDescription> inBus;
    vector<PadDescription> outBus;
    ConnectionGraphDescriptor graph;
    string docString;
    int removePort(const string & portName, bool inputPort);
    int addPort(const string & portName, bool inputPort, const string & unit, const float & defaultValue);
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
    vector<PatchParameterDescriptor> parameters;
};

/* Preset (voice patch + effect patch + parameters) */
// these are managed by JUCE layer

struct PresetParameterDescriptor {
    int index;
    PatchParameterDescriptor p;
};

struct PresetDescriptor {
    PatchDescriptor voice;
    PatchDescriptor effect;
    vector<PresetParameterDescriptor> parameters;
};

/* Functions and aux types */

void designPatch(
    ConnectionGraph &connectionGraph,
    const PatchDescriptor &description,
    const vector<PadDescription>& inBus,
    const vector<PadDescription>& outBus,
    map<string, int> &moduleHandles,
    SynthGraphType type,
    ParameterHandleMap &parameterHandles,
    const ComponentRegister & cp
);

// TODO, designPreset

const char componentSeparator = '.';
const char parameterMarker = '=';
const char componentMarker = '@';

bool moduleNameIsValid(const string& moduleName);
bool moduleTypeIsValid(const string& componentName);
bool componentTypeIsValid(const string& componentName);

// TODO SynthGraphTypeIsValid
// TODO typeIsValid (checks that all uppercase?)

// TODO isParameter, isComponent (that checks first char but ignores the validity at large)

extern const vector<PadDescription> c_effectChainInBus;
extern const vector<PadDescription> c_effectChainOutBus;

extern const vector<PadDescription> c_voiceChainInBus;
extern const vector<PadDescription> c_voiceChainOutBus;

extern const ModuleVariable c_inBus;
extern const ModuleVariable c_outBus;


}
