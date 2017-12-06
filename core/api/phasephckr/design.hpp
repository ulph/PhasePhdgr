#pragma once

#include <utility>
#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <assert.h>

class ConnectionGraph;

#define NYI assert(0)

using namespace std;

namespace PhasePhckr {

class ComponentRegister;

/* Graph stuff */

struct ModuleVariable {
    string name = "";
    string type = "";
    ModuleVariable() {};
    ModuleVariable(const pair<string, string>& kv) : name(kv.first), type(kv.second) {};
    ModuleVariable(const string& n, const string& t) : name(n), type(t) {};
    bool operator ==(ModuleVariable const& other) const {
        return other.name == name && other.type == type;
    }
    bool conflicts(ModuleVariable const& other) const {
        return name == other.name;
    }
};

struct ModulePort {
    string module = "";
    string port = "";
    ModulePort() {}
    ModulePort(const string& m, const string& p) : module(m), port(p) {}
    bool operator ==(ModulePort const& other) const {
        return other.module == module && other.port == port;
    }
    bool operator <(ModulePort const& other) const {
        return other.module < module && other.port < port;
    }
    explicit operator pair<string, string>() const { return make_pair(module, port); } // TODO, prune
};

struct ModulePortConnection {
    ModulePort source;
    ModulePort target;
    bool operator ==(ModulePortConnection const& other) const {
        return source == other.source && target == other.target;
    }
};

struct ModulePortValue {
    ModulePort target;
    float value;
    ModulePortValue(){}
    ModulePortValue(const ModulePort& mp, float v) : target(mp), value(v) {}
    bool conflicts(ModulePortValue const& other) const {
        return target == other.target;
    }
    void setValue(float newValue) { value = newValue; }
    float getValue() const { return value; }
    const ModulePort& getTarget() const { return target; }
    void setTarget(const ModulePort& newTarget) { target = newTarget; }
};

struct ConnectionGraphDescriptor {
    map<string, string> modules;
    vector<ModulePortConnection> connections;
    map<ModulePort, float> values;

    void pruneBusModules();

    void pruneDanglingConnections();

    void pruneInvalidConnections() {
        NYI; // to inBus, from outBus
    }

    void cleanUp() {
        pruneBusModules();
        pruneDanglingConnections();
        pruneInvalidConnections();
    }

    int add(const string& module, const string& type);

    int remove(const string& module) {
        if (!modules.count(module)) return -1;
        modules.erase(module);
        pruneDanglingConnections();
        return 0;
    }

    int rename(const string& module, const string& newModule) {
        NYI; return -1;
    }

    int clone(const string& module, const string& clone) {
        NYI; return -1;
    }

    int clone(const set<string*>& modules) {
        NYI; return -1;
    }

    int createNewComponent(const set<string*>& modules, const string& type) {
        NYI; return -1;
    }

    bool validConnection(const ModulePortConnection& connection);

    int connect(const ModulePortConnection& connection) {
        NYI; return -1;
    }

    int disconnect(const ModulePortConnection& connection, bool all = false);

    int disconnect(const ModulePort& target) {
        NYI; return -1;
    }

    int disconnect(const string& module) {
        NYI; return -1;
    }

    int clearValue(const ModulePortConnection& target) {
        NYI; return -1;
    }

    int clearValues(const string& module) {
        NYI; return -1;
    }

    int setValue(const ModulePortConnection& target) {
        NYI; return -1;
    }

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
    int renamePort(const string & portName, const string & newPortName, bool inputPort) {
        NYI; return -1;
    }

    void pruneUnusedPorts() {
        NYI;
    }

    void cleanUp() {
        pruneUnusedPorts();
        graph.cleanUp();
    }

};

/* Patch */

struct ModulePosition {
    int x = 0;
    int y = 0;
    ModulePosition() {}
    ModulePosition(int x_, int y_) : x(x_), y(y_) {}
    ModulePosition(float x_, float y_) : x((int)(x_ + 0.5f)), y((int)(y_ + 0.5f)) {}
};

struct PatchDescriptor {
    ComponentDescriptor root;
    map<string, ComponentDescriptor> components;
    map<string, ModulePosition> layout; // TODO; move onto ComponentDescriptor?
    vector<PatchParameterDescriptor> parameters;

    int renameComponentType(const string& type, const string& newType) {
        NYI; return -1;
    }

    int removeComponentType(const string& type) {
        NYI; return -1;
    }

    int addComponentType(const string& type, const ComponentDescriptor& descriptor) {
        NYI; return -1;
    }

    int getComponentType(const string& type, const ComponentDescriptor& descriptor) {
        NYI; return -1;
    }

    void pruneUnusedComponents();
    void pruneLayout();

    void cleanUp() {
        pruneUnusedComponents();
        pruneLayout();
        root.cleanUp();
        for (auto& kv : components) {
            kv.second.cleanUp();
        }
    }

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
