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

struct ComponentDescriptor {
    vector<PadDescription> inBus;
    vector<PadDescription> outBus;
    ConnectionGraphDescriptor graph;
    string docString;
    int removePort(const string & portName, bool inputPort);
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

extern const vector<PadDescription> c_effectChainInBus;
extern const vector<PadDescription> c_effectChainOutBus;

extern const vector<PadDescription> c_voiceChainInBus;
extern const vector<PadDescription> c_voiceChainOutBus;

extern const ModuleVariable c_inBus;
extern const ModuleVariable c_outBus;


}
