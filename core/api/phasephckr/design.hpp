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
        return pair<string, string>(other) < pair<string, string>(*this);
    }
    explicit operator pair<string, string>() const { return make_pair(module, port); }
};

struct ModulePortConnection {
    ModulePort source;
    ModulePort target;
    ModulePortConnection() {}
    ModulePortConnection(const ModulePort& src, const ModulePort& trg) : source(src), target(trg) {}
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
};

struct ConnectionGraphDescriptor {
    map<string, string> modules;
    vector<ModulePortConnection> connections;
    map<ModulePort, float> values;

    void pruneBusModules();

    void pruneDanglingConnections();

    void cleanUp() {
        pruneBusModules();
        pruneDanglingConnections();
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

    int connect(const ModulePortConnection& connection);

    int disconnect(const ModulePortConnection& connection, bool all = false);

    int disconnect(const ModulePort& endpoint, bool isInput);

    int clearValue(const ModulePort& target);

    int getValue(const ModulePort& target, float& value);

    int setValue(const ModulePort& target, float value);

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
    void cleanUp() {
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
    // TODO - globalComponents and localComponents?
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

bool portNameIsValid(const string& portName);
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
