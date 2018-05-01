#pragma once

#include <utility>
#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <assert.h>
#include <list>

class ConnectionGraph;

#define PP_NYI assert(0)

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
    bool operator==(const ModuleVariable& rhs) const {
        return std::tie(name, type) == std::tie(rhs.name, rhs.type);
    }
};

struct ModulePort {
    string module = "";
    string port = "";
    ModulePort() {}
    ModulePort(const string& m, const string& p) : module(m), port(p) {}
    bool operator==(const ModulePort& rhs) const {
        return std::tie(module, port) == std::tie(rhs.module, rhs.port);
    }
    bool operator<(const ModulePort& rhs) const {
        return std::tie(module, port) < std::tie(rhs.module, rhs.port);
    }
    explicit operator pair<string, string>() const { return make_pair(module, port); }
};

struct ModulePortConnection {
    ModulePort source;
    ModulePort target;
    ModulePortConnection() {}
    ModulePortConnection(const ModulePort& src, const ModulePort& trg) : source(src), target(trg) {}
    bool operator==(const ModulePortConnection& rhs) const {
        return std::tie(target, source) == std::tie(rhs.target, rhs.source);
    }
    bool operator<(const ModulePortConnection& rhs) const {
        return std::tie(target, source) < std::tie(rhs.target, rhs.source);
    }
    explicit operator pair<ModulePort, ModulePort>() const { return make_pair(source, target); }
};

struct ModulePortValue {
    ModulePort target;
    float value;
    ModulePortValue(){}
    ModulePortValue(const ModulePort& mp, float v) : target(mp), value(v) {}
    bool operator==(const ModulePortValue& rhs) const {
        return std::tie(target, value) == std::tie(rhs.target, rhs.value);
    }
};

struct ModulePosition {
    int x = 0;
    int y = 0;
    ModulePosition() {}
    ModulePosition(int x_, int y_) : x(x_), y(y_) {}
    ModulePosition(float x_, float y_) : x((int)(x_ + 0.5f)), y((int)(y_ + 0.5f)) {}
    bool operator==(const ModulePosition& rhs) const {
        return std::tie(x, y) == std::tie(rhs.x, rhs.y);
    }
};

struct ConnectionGraphDescriptor {
    map<string, string> modules;
    vector<ModulePortConnection> connections;
    map<ModulePort, float> values;
    map<string, ModulePosition> layout;
    bool operator==(const ConnectionGraphDescriptor& rhs) const {
        // TODO, probably just exclude layout from comparison
        return std::tie(modules, connections, values, layout) == std::tie(rhs.modules, rhs.connections, rhs.values, rhs.layout);
    }

    std::set<string> getTypes() {
        std::set<string> types;
        for (const auto& m : modules) types.insert(m.second);
        return types;
    }

    void pruneBusModules();
    void pruneDanglingConnections();
    void pruneOrphanValues();
    void pruneLayout();

    void cleanUp() {
        pruneBusModules();
        pruneDanglingConnections();
        pruneOrphanValues();
        pruneLayout();
    }

    int add(const string& module, const string& type);
    int remove(const string& module);
    int rename(const string& module, const string& newModule);

    int clone(const string& module, const string& clone) {
        PP_NYI; return -1;
    }

    int clone(const set<string*>& modules) {
        PP_NYI; return -1;
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
    bool operator==(const PadDescription& rhs) const {
        return std::tie(name, unit, defaultValue) == std::tie(rhs.name, rhs.unit, rhs.defaultValue);
    }
    PadDescription(){}
    PadDescription(string n, string u, float v) : name(n), unit(u), defaultValue(v){}
};

struct ParameterValueDescriptor{
    float val = 0.0f;
    float min = 0.0f;
    float max = 1.0f;
    bool operator==(const ParameterValueDescriptor& rhs) const {
        return std::tie(val, min, max) == std::tie(rhs.val, rhs.min, rhs.max);
    }
};

struct PatchParameterDescriptor {
    string id;
    ParameterValueDescriptor v;
    bool operator==(const PatchParameterDescriptor& rhs) const {
        return std::tie(id, v) == std::tie(rhs.id, rhs.v);
    }
};

struct ComponentDescriptor {
    vector<PadDescription> inBus;
    vector<PadDescription> outBus;
    ConnectionGraphDescriptor graph;
    string docString;
    bool operator==(const ComponentDescriptor& rhs) const {
        return std::tie(inBus, outBus, graph, docString) == std::tie(rhs.inBus, rhs.outBus, rhs.graph, rhs.docString);
    }
    int removePort(const string & portName, bool inputPort);
    int addPort(const string & portName, bool inputPort, const string & unit, const float & defaultValue);
    int renamePort(const string & portName, const string & newPortName, bool inputPort);
    int changePortUnit(const string & portName, const string & newUnit);
    int changePortValue(const string & portName, float newValue);
    bool hasPort(const string & portName, bool inputPort) const;
    int getPort(const string & portName, PadDescription& result, bool inputPort) const;
    void cleanUp() {
        graph.cleanUp();
    }
    void componentTypeWasRenamed(const string& type, const string& newType);
    void componentTypePortWasRenamed(const string& type, const string& port, const string& newPort, bool inputPort);
    int addModule(const string& type, string& name);
};

/* Patch */

struct ComponentBundle {
private:
    map<string, ComponentDescriptor> components;
public:
    bool operator==(const ComponentBundle& rhs) const {
        return std::tie(components) == std::tie(rhs.components);
    }
    bool has(const string& type) const { return components.count(type); }
    const ComponentDescriptor& get(const string& type) const { return components.at(type); }
    ComponentDescriptor * getPointer(const string& type) { return components.count(type) ? &components.at(type) : nullptr; } // TODO, guard with friend access
    const map<string, ComponentDescriptor>& getAll() const { return components; }
    void setAll(const map<string, ComponentDescriptor>& components_) { components = components_; }
    int create(ComponentDescriptor* rootComponent, const set<string>& modules, string& type);
    int remove(const string& type);
    int add(string& type, const ComponentDescriptor& descriptor, bool resolveNameConflict);
    int set(const string& type, const ComponentDescriptor& descriptor); // potentially braking graphs
    int rename(ComponentDescriptor* rootComponent, const string& type, const string& newType); // TODO, list of roots
    int renamePort(ComponentDescriptor* rootComponent, const string& type, const string& port, const string& newPort, bool inputPort); // TODO, list of roots

    int removePort(const string& type, const string& port, bool inputPort) {
        if (!has(type)) return -1;
        return components.at(type).removePort(port, inputPort);
    }

    int setDocString(const string& type, const string& docString) {
        if (!has(type)) return -1;
        components.at(type).docString = docString;
        return 0;
    }

    int addPort(const string& type, const string& portName, bool inputPort, const string& unit, float defaultValue) {
        if (!has(type)) return -1;
        return components.at(type).addPort(portName, inputPort, unit, defaultValue);
    }

    int setPortUnit(const string& type, const string& port, const string& unit, bool inputPort) {
        if (!has(type)) return -1;
        if (!inputPort) return -2; // NYI
        return components.at(type).changePortUnit(port, unit);
    }

    int setPortValue(const string& type, const string& port, float value) {
        if (!has(type)) return -1;
        return components.at(type).changePortValue(port, value);
    }

    int setLayout(const string& type, const map<string, ModulePosition> & layout) {
        if (!has(type)) return -1;
        components.at(type).graph.layout = layout;
        return 0;
    }

    void cleanUp() {
        for (auto& kv : components) {
            kv.second.cleanUp();
        }
    }

    void appendToUnion(const map<string, ComponentDescriptor> &other, std::set<string>& referenced) {
        auto numReferenced = referenced.size();

        // find all top-level references and in turn, what they refer
        for (const auto& c : components) {
            if (referenced.count(c.first)) {
                for (const auto& kv : c.second.graph.modules) {
                    referenced.insert(ModuleVariable(kv).type);
                }
            }
            // ignore the rest
        }

        // add them in if missing
        for (auto& kv : other) {
            if (!components.count(kv.first) && referenced.count(kv.first)) {
                components[kv.first] = kv.second;
            }
        }

        // recurse if we found stuff
        if (referenced.size() > numReferenced)
            appendToUnion(other, referenced);
    }

    void reduceToComplement(const map<string, ComponentDescriptor> &other) {
        for (auto& kv : other) {
            if (components.count(kv.first) && components.at(kv.first) == kv.second) {
                components.erase(kv.first);
            }
        }
    }

    void appendToUnion(const ComponentBundle & other, std::set<string>& referenced) {
        appendToUnion(other.getAll(), referenced);
    }

    void reduceToComplement(const ComponentBundle & other) {
        reduceToComplement(other.getAll());
    }
};

struct PatchDescriptor {
    ComponentDescriptor root;
    ComponentBundle componentBundle;
    vector<PatchParameterDescriptor> parameters;
    bool operator==(const PatchDescriptor& rhs) const {
        return std::tie(root, componentBundle, parameters) == std::tie(rhs.root, rhs.componentBundle, rhs.parameters);
    }
    void cleanUp() {
        root.cleanUp();
        componentBundle.cleanUp();
    }
};

/* Preset (voice patch + effect patch + parameters) */

enum NoteStealPolicy {
    NoteStealPolicyNone = 0, // never steal an active voice
    NoteStealPolicyOldest, // steal the oldest active voice
    NoteStealPolicyLowestRMS, // steal the most quiet active voice
    NoteStealPolicyIfLower, // steal the lowest voice (in note number) - when released the lowest stolen voice get revived
    NoteStealPolicyIfHigher, // steal the highest voice (in note number) - when released the highest stolen voice get revived
    NoteStealPolicyYoungest, // steal the youngest active voice
    NoteStealPolicyClosest,
//    NoteStealPolicyHigher,
//    NoteStealPolicyLower,
    NoteStealPolicyAuto = 99, // best match given NoteActivationPolicy and polyphony
};

enum NoteReactivationPolicy {
    NoteReactivationPolicyNone = 0, // do not reactivate stolen voices
    NoteReactivationPolicyLast, // reactivate the last pressed key (that is stolen)
    NoteReactivationPolicyHighest, // reactivate the highest stolen voice
    NoteReactivationPolicyLowest, // reactivate the lowest stolen voice
    NoteReactivationPolicyFirst, // reactivate the first pressed key (that is stolen)
    NoteReactivationPolicyClosest,
//    NoteReactivationPolicyLower,
//    NoteReactivationPolicyHigher,
    NoteReactivationPolicyAuto = 99, // best match given NoteStealPolicy and polyphony
};

enum LegatoMode {
    LegatoModeRetrigger = 0,
    LegatoModeUpdateVelocity,
    LegatoModeFreezeVelocity,
    LegatoModeReleaseVelocity,
    LegatoModeRetriggerReleaseVelocity,
};

enum NoteActivationPolicy {
    // TODO, change up to be
    // FirstInactive
    // RoundRobin
    // ...
    // and permutate whether to tolerate non-silent or not.
    NoteActivationPolicyOnlySilent = 0, // any silent inactive, but nothing else
    NoteActivationPolicyOldest, // any silent voice, fall back to oldest not silent (longest in gate off)
};

// TODO sustainPolicy 
// - Auto
// - trigger same
// - trigger new

const auto c_NoteActivationPolicyDefault = NoteActivationPolicyOldest;
const auto c_NoteStealPolicyDefault = NoteStealPolicyAuto;
const auto c_NoteReactivationPolicyDefault = NoteReactivationPolicyAuto;
const auto c_LegatoModeDefault = LegatoModeFreezeVelocity;

struct PresetSettings {
    NoteActivationPolicy noteActivationPolicy = c_NoteActivationPolicyDefault; // how to select which inactive voice to activate
    NoteStealPolicy noteStealPolicy = c_NoteStealPolicyDefault; // if, and how, to steal voices
    NoteReactivationPolicy noteReactivationPolicy = c_NoteReactivationPolicyDefault; // if, and how, to re-activate stolen notes
    LegatoMode legatoMode = c_LegatoModeDefault; // if stealing a voice, retrigger or legato - with frozen or updated velocity

    int polyphony = 16; // how many simultaneous voices to process
    bool multicore = true; // process each voice on it's own thread

    NoteStealPolicy getNoteStealPolicy();
    NoteReactivationPolicy getNoteReactivationPolicy();
};

enum SynthGraphType {
    UNDEFINED = 0,
    VOICE = 1,
    EFFECT = 2
};

struct PresetParameterDescriptor {
    int index;
    SynthGraphType type = UNDEFINED;
    PatchParameterDescriptor p; // TODO, change so that it only contains the string id
};

struct PresetDescriptor {
    PatchDescriptor voice;
    PatchDescriptor effect;   
    // ... TODO, move any combination/separation logic from JUCE to this struct!
    vector<PresetParameterDescriptor> parameters; // need to store voice and effect _positions_ jointly, TODO move position onto e/v and handle conflicts here instead!
    PresetSettings settings;
};

/* Functions and aux types */

typedef map<int, PatchParameterDescriptor> ParameterHandleMap;

void designPatch(
    ConnectionGraph &connectionGraph,
    const PatchDescriptor &description,
    const ComponentBundle &componentBundle,
    const vector<PadDescription>& inBus,
    const vector<PadDescription>& outBus,
    map<string, int> &moduleHandles,
    SynthGraphType type,
    ParameterHandleMap &parameterHandles,
    const ComponentRegister & cp
);

// TODO, designPreset

const char scopeSeparator = '.';
const char parameterMarker = '=';
const char componentMarker = '@';

bool portIsValid(const string& portName);
bool nameIsValid(const string& moduleName, bool allowScope);
bool typeIsValid(const string& componentName, bool allowScope);

bool componentTypeIsValid(const string& componentName, bool allowScope);
bool parameterTypeIsValid(const string& componentName, bool allowScope);
bool genericTypeIsValid(const string& componentName, bool allowScope);

string nameFromType(const string& type);

extern const vector<PadDescription> c_effectChainInBus;
extern const vector<PadDescription> c_effectChainOutBus;

extern const vector<PadDescription> c_voiceChainInBus;
extern const vector<PadDescription> c_voiceChainOutBus;

extern const ModuleVariable c_inBus;
extern const ModuleVariable c_outBus;

}
