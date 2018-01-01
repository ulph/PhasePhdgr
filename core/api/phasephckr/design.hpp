#pragma once

#include <utility>
#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <assert.h>

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
    bool operator ==(ModuleVariable const& other) const {
        return other.name == name && other.type == type;
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
};

struct ModulePosition;

struct ConnectionGraphDescriptor {
    map<string, string> modules;
    vector<ModulePortConnection> connections;
    map<ModulePort, float> values;

    void pruneBusModules();
    void pruneDanglingConnections();
    void pruneOrphanValues();

    void cleanUp() {
        pruneBusModules();
        pruneDanglingConnections();
        pruneOrphanValues();
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
    PadDescription(){}
    PadDescription(string n, string u, float v) : name(n), unit(u), defaultValue(v){}
};

struct ParameterValueDescriptor{
    float val = 0.0f;
    float min = 0.0f;
    float max = 1.0f;
};

struct PatchParameterDescriptor {
    string id;
    ParameterValueDescriptor v;
};

struct ComponentDescriptor {
    vector<PadDescription> inBus;
    vector<PadDescription> outBus;
    ConnectionGraphDescriptor graph;
    string docString;
    map<string, ModulePosition> layout;
    int removePort(const string & portName, bool inputPort);
    void pruneLayout();
    int addPort(const string & portName, bool inputPort, const string & unit, const float & defaultValue);
    int renamePort(const string & portName, const string & newPortName, bool inputPort);
    int changePortUnit(const string & portName, const string & newUnit);
    int changePortValue(const string & portName, float newValue);
    bool hasPort(const string & portName, bool inputPort);
    int getPort(const string & portName, PadDescription& result, bool inputPort);
    void cleanUp() {
        pruneLayout();
        graph.cleanUp();
    }
    void componentTypeWasRenamed(const string& type, const string& newType);
    void componentTypePortWasRenamed(const string& type, const string& port, const string& newPort, bool inputPort);
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
    // TODO - globalComponents and localComponents? Benefits are easier plumbing of data, easy to report what is overlapping etc
    map<string, ComponentDescriptor> components;
    vector<PatchParameterDescriptor> parameters;

    int createNewComponentType(ComponentDescriptor* rootComponent, const set<string>& modules, string& type);
    int addComponentType(string& type, const ComponentDescriptor& descriptor, bool resolveNameConflict=false);
    int renameComponentType(const string& type, const string& newType);
    int renameComponentTypePort(const string& type, const string& port, const string& newPort, bool inputPort);
    int removeComponentType(const string& type);

    void pruneUnusedComponents();

    void cleanUp() {
        pruneUnusedComponents();
        root.cleanUp();
        for (auto& kv : components) {
            kv.second.cleanUp();
        }
    }

};

/* Preset (voice patch + effect patch + parameters) */

enum NoteStealPolicy {
    NoteStealPolicyDoNotSteal = 0, // never steal an active voice
    NoteStealPolicyStealOldest, // steal the oldest active voice
    NoteStealPolicyStealLowestRMS, // steal the most quiet active voice
    NoteStealPolicyStealIfLower, // steal the lowest voice (in note number) - when released the lowest stolen voice get revived
    NoteStealPolicyStealIfHigher, // steal the highest voice (in note number) - when released the highest stolen voice get revived
//  NoteStealPolicyYoungest,
//  NoteStealPolicyClosest,
//  NoteStealPolicyHigher,
//  NoteStealPolicyLower,
//  NoteStealAuto, // best match given polyphony etc
};

enum NoteReactivationPolicy {
    NoteReactivationPolicyDoNotReactivate = 0, // do not reactivate stolen voices
    NoteReactivationPolicyLast, // reactivate the last pressed key (that is stolen)
    NoteReactivationPolicyHighest, // reactivate the highest stolen voice
    NoteReactivationPolicyLowest, // reactivate the lowest stolen voice
//  NoteReactivationPolicyFirst,
//  NoteReactivationPolicyClosest,
//  NoteReactivationPolicyLower,
//  NoteReactivationPolicyHigher,
//  NoteReactivationPolicyAuto, // best match given NoteStealPolicy and polyphony ...
};

enum LegatoMode {
    LegatoModeRetrigger = 0,
    LegatoModeUpdateVelocity,
    LegatoModeFreezeVelocity,
//    LegatoModeReleaseVelocity,
};

enum NoteActivationPolicy {
    NoteActivationPolicyOnlySilent = 0, // pick any silent inactive, but nothing else
    NoteActivationPolicyPreferOldestSilent, // pick the oldest inactive and silent, and then pick oldest inactive
    NoteActivationPolicyPreferOldestNotSilent, // pick the oldest inactive, but not silent, before anything else
    NoteActivationPolicyPreferYoungestNotSilent, // pick the youngest inactive, but not silent, before anything else
};

struct PresetSettings {
    NoteActivationPolicy noteActivationPolicy = NoteActivationPolicyPreferOldestSilent; // how to select which inactive voice to activate

    NoteStealPolicy noteStealPolicy = NoteStealPolicyDoNotSteal; // if, and how, to steal voices
    NoteReactivationPolicy noteReactivationPolicy = NoteReactivationPolicyDoNotReactivate; // if, and how, to re-activate stolen notes
    LegatoMode legatoMode = LegatoModeRetrigger; // if stealing a voice, retrigger or legato - with frozen or updated velocity

    int polyphony = 16; // how many simultaneous voices to process
    bool multicore = true; // process each voice on it's own thread
};

enum SynthGraphType {
    UNDEFINED = 0,
    VOICE = 1,
    EFFECT = 2
};

struct PresetParameterDescriptor {
    int index;
    SynthGraphType type = UNDEFINED;
    PatchParameterDescriptor p;
};

struct PresetDescriptor {
    PatchDescriptor voice;
    PatchDescriptor effect;
    vector<PresetParameterDescriptor> parameters;
    PresetSettings settings;
};

/* Functions and aux types */

typedef map<int, PatchParameterDescriptor> ParameterHandleMap;

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

const char c_pathSeparator = '/';

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
