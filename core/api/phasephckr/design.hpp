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
    string name;
    string type;
    bool operator ==(ModuleVariable const& other) const {
        return other.name == name && other.type == type;
    }
    bool conflicts(ModuleVariable const& other) const {
        return name == other.name;
    }
};

struct ModulePort {
    string module;
    string port;
    bool operator ==(ModulePort const& other) const {
        return other.module == module && other.port == port;
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
    bool conflicts(ModulePortValue const& other) const {
        return target == other.target;
    }
    void setValue(float newValue) { value = newValue; }
    float getValue() const { return value; }
    const ModulePort& getTarget() const { return target; }
    void setTarget(const ModulePort& newTarget) { target = newTarget; }
};

struct ConnectionGraphDescriptor {
    vector<ModuleVariable> modules; // TODO, make a set instead, where existance uses only module name
    vector<ModulePortConnection> connections;
    vector<ModulePortValue> values; // TODO, make a set, where existance is based on ModulePort

    void sanitizeModuleNames() {
        NYI;
    }

    void pruneDuplicatModules() {
        NYI;
    }

    void pruneDuplicateValues() {
        NYI;
    }

    void pruneDanglingConnections() {
        NYI; // remember inBus and outBus are not present in modules
    }

    void cleanUp() {
        pruneDuplicatModules();
        pruneDuplicateValues();
        pruneDanglingConnections();
        sanitizeModuleNames();
    }

    int get(const string& module, const ModuleVariable& descriptor) {
        NYI;
    }

    int add(const string& module, const string& type) {
        NYI;
    }

    int remove(const string& module) {
        NYI;
    }

    int clone(const string& module, const string& clone) {
        NYI;
    }

    int clone(const set<string*>& modules) {
        NYI;
    }

    int createNewComponent(const set<string*>& modules, const string& type) {
        NYI;
    }

    int connect(const ModulePortConnection& from, const ModulePortConnection& to) {
        NYI;
    }

    int disconnect(const ModulePortConnection& from, const ModulePortConnection& to, bool all=false) {
        NYI;
    }

    int disconnect(const string& module) {
        NYI;
    }

    int clearValue(const ModulePortConnection& target) {
        NYI;
    }

    int clearValues(const string& module) {
        NYI;
    }

    int setValue(const ModulePortConnection& target) {
        NYI;
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
        NYI;
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
    int x;
    int y;
};

struct PatchDescriptor {
    ComponentDescriptor root;
    map<string, ComponentDescriptor> components;
    map<string, ModulePosition> layout; // TODO; move onto ComponentDescriptor?
    vector<PatchParameterDescriptor> parameters;

    void sanitizeComponentTypes() {
        NYI;
    }

    int renameComponentType(const string& type) {
        NYI;
    }

    int removeComponentType(const string& type) {
        NYI;
    }

    int addComponentType(const string& type, const ComponentDescriptor& descriptor) {
        NYI;
    }

    int getComponentType(const string& type, const ComponentDescriptor& descriptor) {
        NYI;
    }

    void pruneUnusedComponents() {
        set<string> usedTypes;
        for (const auto& m : root.graph.modules) {
            usedTypes.insert(m.type);
        }
        auto it = components.begin();
        while (it != components.end()) {
            if (!usedTypes.count(it->first)) {
                it = components.erase(it);
            }
            else {
                it++;
            }
        }
    }

    void cleanUp() {
        pruneUnusedComponents();
        sanitizeComponentTypes();
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
