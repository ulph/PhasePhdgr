#include "phasephckr/components.hpp"
#include "phasephckr/design.hpp"

#include "connectiongraph.hpp"
#include <set>
#include <list>
#include <queue>
#include <locale>

#include "busmodules.hpp"

#include <assert.h>

using namespace std;

namespace PhasePhckr {

void designChain(
    ConnectionGraph &g,
    const PatchDescriptor &pd,
    ComponentDescriptor &cd,
    map<string, int> &moduleHandles,
    SynthGraphType type,
    ParameterHandleMap &parameterHandles,
    int depth
);

bool checkName(const string& name) {
    if (!moduleNameIsValid(name)) {
        cerr << "Error: \"" << name << "\" is not a valid Module name!" << endl;
        return false;
    }
    return true;
}

bool checkModule(const ModuleVariable& m){
    if(!moduleTypeIsValid(m.type)){
        cerr << "Error: \"" << m.type << "\" is not a valid Module type!" << endl;
        return false;
    }
    return checkName(m.name);
}

bool checkComponent(const ModuleVariable& m, const PatchDescriptor& pd) {
    if (!pd.components.count(m.type)) {
        cerr << "Error: \"" << m.type << "\" is unknown Component type!" << endl;
        return false;
    }
    if (!componentTypeIsValid(m.type)) {
        cerr << "Error: \"" << m.type << "\" is not a valid Component type!" << endl;
        return false;
    }
    return checkName(m.name);
}

bool unpackComponent(
    ConnectionGraph &g,
    const ModuleVariable &mv,
    const PatchDescriptor &pd,
    ComponentDescriptor &parent,
    ComponentDescriptor &current,
    map<string, int> &moduleHandles,
    SynthGraphType type,
    ParameterHandleMap &parameterHandles,
    int depth
) {

    assert(depth > 0);

    // find the component definition
    cout << "component " << mv.name << " " << mv.type << endl;

    // move component subgraph onto the mv's "scope"
    const string pfx = mv.name + componentSeparator;

    auto inBus = new BusModule(current.inBus, true);
    auto outBus = new BusModule(current.outBus, false);
    string inBusName = pfx+c_inBus.name;
    string outBusName = pfx+c_outBus.name;
    moduleHandles[inBusName] = g.addCustomModule(inBus);
    moduleHandles[outBusName] = g.addCustomModule(outBus);
    cout << moduleHandles[inBusName] << " : " << " " << inBusName << std::endl;
    cout << moduleHandles[outBusName] << " : " << " " << outBusName << std::endl;

    // find any connections to and from this component and "re-route" to inBus/outBus
    for(auto &c : parent.graph.connections){
        if(c.source.module == mv.name){
           c.source.module = outBusName;
        }
        if(c.target.module == mv.name){
            c.target.module = inBusName;
        }
    }

    // copy all values from inBus onto values
    for(const auto &i : current.inBus){
        current.graph.values[ModulePort(c_inBus.name, i.name)] = i.defaultValue;
    }

    // find any values set on this component and reroute to inBus
    map<ModulePort, float> newParentValuesMap;
    for(auto &kv : parent.graph.values){
        ModulePortValue mpv(kv.first, kv.second);
        if (mpv.target.module == mv.name) {
            mpv.target.module = inBusName;
        }
        newParentValuesMap[mpv.target] = mpv.value;
    }
    parent.graph.values = newParentValuesMap;

    // prefix internals accordingly
    map<string, string> newModulesMap;
    for (const auto &kv : current.graph.modules) {
        const ModuleVariable m(kv);
        if (!checkModule(m)) continue;
        newModulesMap[pfx + kv.first] = kv.second;
    }
    current.graph.modules = newModulesMap;

    map<ModulePort, float> newValuesMap;
    for (const auto &kv : current.graph.values) {
        ModulePortValue mpv(kv.first, kv.second);
        mpv.target.module = pfx + mpv.target.module;
        newValuesMap[mpv.target] = mpv.value;
    }
    current.graph.values = newValuesMap;

    for (auto &c : current.graph.connections) {
        c.source.module = pfx + c.source.module;
        c.target.module = pfx + c.target.module;
    }

    designChain(g, pd, current, moduleHandles, type, parameterHandles, depth);

    return true;
}


void designChain(
    ConnectionGraph &g,
    const PatchDescriptor &pd,
    ComponentDescriptor &cd,
    map<string, int> &moduleHandles,
    SynthGraphType type,
    ParameterHandleMap &parameterHandles,
    int depth
){

    set<string> components;

    // find any components (module bundles) and unpack them
    for (const auto &kv : cd.graph.modules) {
        const ModuleVariable m(kv);
        if (m.type.front() == componentMarker) {
            if (!checkComponent(m, pd)) continue;
            ComponentDescriptor child_cd = pd.components.at(m.type);
            if(unpackComponent(g, m, pd, cd, child_cd, moduleHandles, type, parameterHandles, depth+1)){
                components.insert(m.name);
            }
        }
    }

    // create the modules and store their handles
    for (const auto &kv : cd.graph.modules) {
        const ModuleVariable m(kv);
        if(depth == 0 && !checkModule(m)) continue;
        if (moduleHandles.count(m.name) > 0) {
            cerr << "Error: \"" << m.name << "\" name dupe! (modules)" << endl;
        }
        if (components.count(m.name)) continue;

        int handle = g.addModule(m.type);
        moduleHandles[m.name] = handle;
        cout << handle << " : " << " " << m.name << std::endl;
        if(m.type.front() == parameterMarker){
            PatchParameterDescriptor prm;
            prm.min = 0.f;
            prm.max = 1.f;
            prm.val = 0.f;
            prm.type = type;
            prm.id = m.name;
            parameterHandles[handle] = prm;
        }
    }

    // iterate over the connections
    for (const auto &c : cd.graph.connections) {
        int hFrom = moduleHandles.count(c.source.module) > 0 ? moduleHandles.at(c.source.module) : -2;
        int hTo = moduleHandles.count(c.target.module) > 0 ? moduleHandles.at(c.target.module) : -2;
        cout << c.source.module << ":" << c.source.port << " -> " << c.target.module << ":" << c.target.port << endl;
        if (hFrom < 0 || hTo < 0) {
            cerr << "Error: \"" << c.source.module << " -> " << c.target.module << "\" invalid. ";
            if (hFrom < 0) {
                cerr << " \"" << c.source.module << "\" not found! ";
            }
            if (hTo < 0) {
                cerr << " \"" << c.target.module << "\" not found! ";
            }
            cerr << endl;
        }
        else {
            g.connect(hFrom, c.source.port, hTo, c.target.port);
        }
    }

    // set default values provided
    for (const auto &kv : cd.graph.values) {
        const ModulePortValue v(kv.first, kv.second);
        if (components.count(v.target.module)) continue;
        int h = moduleHandles.count(v.target.module) > 0 ? moduleHandles.at(v.target.module) : -2;
        cout << v.target.module << ":" << v.target.port << " = " << v.value << endl;
        if (h < 0) {
            cerr << "Error: " << v.target.module << " not found! (values)" << endl;
        }
        else if (parameterHandles.count(h)) {
            // bubble up any values set onto parameters as actual parameter data
            if (v.target.port == "value")   parameterHandles[h].val = v.value;
            else if(v.target.port == "min") parameterHandles[h].min = v.value;
            else if(v.target.port == "max") parameterHandles[h].max = v.value;
        }
        else {
            g.setInput(h, v.target.port, v.value);
        }
    }

}


void designPatch(
    ConnectionGraph &connectionGraph,
    const PatchDescriptor &p_,
    const vector<PadDescription>& inBus,
    const vector<PadDescription>& outBus,
    map<string, int> &moduleHandles,
    SynthGraphType type,
    ParameterHandleMap &parameterHandles,
    const ComponentRegister & cp
) {

    PatchDescriptor p = p_;

    // copy over all global components
    for (const auto & kv : cp.all()){
        if(!p.components.count(kv.first)){
            p.components[kv.first] = kv.second;
        }
        else{
            cerr << "Warning: Component \"" << kv.first << "\" is globally defined and in patch! Consider renaming the patch defined component." << endl;
        }
    }

    p.root.graph.pruneBusModules();

    // create the special inBus and outBus modules
    auto inBus_ = new BusModule(inBus, true);
    auto outBus_ = new BusModule(outBus, false);
    moduleHandles[c_inBus.name] = connectionGraph.addCustomModule(inBus_);
    moduleHandles[c_outBus.name] = connectionGraph.addCustomModule(outBus_);
    cout << moduleHandles[c_inBus.name] << " : " << " " << c_inBus.name << std::endl;
    cout << moduleHandles[c_outBus.name] << " : " << " " << c_outBus.name << std::endl;

    // begin parsing from root component
    designChain(
        connectionGraph,
        p,
        p.root,
        moduleHandles,
        type,
        parameterHandles,
        0
    );

    map<string, const PatchParameterDescriptor *> patchParams;
    for (const auto& pd : p_.parameters) {
        patchParams[pd.id] = &pd;
    }

    for (auto& kv : parameterHandles) {
        auto& pd = kv.second;
        if (patchParams.count(pd.id)) pd = *patchParams.at(pd.id);
    }

}

void PatchDescriptor::pruneUnusedComponents() {
    set<string> usedTypes;
    for (const auto& kv : root.graph.modules) {
        const ModuleVariable m(kv);
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

void ComponentDescriptor::pruneLayout() {
    set<string> usedNames;
    usedNames.insert(c_inBus.name);
    usedNames.insert(c_outBus.name);
    for (const auto& kv : graph.modules) {
        const ModuleVariable m(kv);
        usedNames.insert(m.name);
    }
    auto it = layout.begin();
    while (it != layout.end()) {
        if (!usedNames.count(it->first)) {
            it = layout.erase(it);
        }
        else {
            it++;
        }
    }
}

bool ConnectionGraphDescriptor::validConnection(const ModulePortConnection& connection) {
    const auto& src = connection.source.module;
    const auto& trg = connection.target.module;
    bool validSource = modules.count(src) || src == c_inBus.name;
    bool validTarget = modules.count(trg) || trg == c_outBus.name;
    return validSource && validTarget;
}

int ConnectionGraphDescriptor::connect(const ModulePortConnection& connection) {
    if (!modules.count(connection.source.module)) return -1;
    if (!modules.count(connection.target.module)) return -2;
    connections.push_back(connection);
    return 0;
}

int ConnectionGraphDescriptor::disconnect(const ModulePortConnection& connection, bool all) {
    bool disconnected = false;
    auto it = connections.begin();
    while(it != connections.end()) {
        if (*it == connection) {
            it = connections.erase(it);
            if (!all) return 0;
            disconnected = true;
        }
        else
            ++it;
    }
    return disconnected ? 0 : -1;
}

int ConnectionGraphDescriptor::clearValue(const ModulePort& target) {
    if (!values.count(target)) return -1;
    values.erase(target);
    return 0;
}

int ConnectionGraphDescriptor::setValue(const ModulePort& target, float value) {
    if (!modules.count(target.module)) return -1;
    values[target] = value;
    return 0;
}

int ConnectionGraphDescriptor::getValue(const ModulePort& target, float& value) {
    if (!values.count(target)) return -1;
    value = values.at(target);
    return 0;
}

int ConnectionGraphDescriptor::disconnect(const ModulePort& endpoint, bool isInput) {
    auto sz = connections.size();
    auto it = connections.begin();
    while (it != connections.end()) {
        if (isInput && it->target == endpoint) it = connections.erase(it);
        else if (!isInput && it->source == endpoint) it = connections.erase(it);
        else ++it;
    }
    return sz > connections.size() ? 0 : -1;
}

void ConnectionGraphDescriptor::pruneDanglingConnections() {
    auto it = connections.begin();
    while (it != connections.end()) {
        if (!validConnection(*it))
            it = connections.erase(it);
        else
            ++it;
    }
}

void ConnectionGraphDescriptor::pruneBusModules() {
    vector<string> toPrune = { c_inBus.name, c_outBus.name };
    auto it = modules.begin();
    while (it != modules.end()) {
        bool prunedStuff = false;
        for (const auto& m : toPrune) {
            if (it->first == m) {
                it = modules.erase(it);
                prunedStuff = true;
            }
        }
        if (!prunedStuff) ++it;
    }
}

int ConnectionGraphDescriptor::add(const string& module, const string& type) {
    if (!moduleNameIsValid(module)) return -1;
    if (!moduleTypeIsValid(type)) return -2;
    if (modules.count(module)) return -3;
    modules[module] = type;
    return 0;
}

int ConnectionGraphDescriptor::remove(const string& module) {
    if (!modules.count(module)) return -1;
    modules.erase(module);
    pruneDanglingConnections();
    return 0;
}

int ConnectionGraphDescriptor::rename(const string& module, const string& newModule, map<string, ModulePosition> *layout) {
    if (!modules.count(module)) return -1;
    if (modules.count(newModule)) return -2;
    if (!moduleNameIsValid(newModule)) return -3;

    auto t = modules.at(module);
    modules.erase(module);
    modules[newModule] = t;

    for (auto& c : connections) {
        if (c.source.module == module) c.source.module = newModule;
        else if (c.target.module == module) c.target.module = newModule;
    }

    set<ModulePort> ks;
    for (const auto& kv : values) {
        if (kv.first.module == module) ks.insert(kv.first);
    }
    for (auto& k : ks) {
        auto v = values[k];
        values.erase(k);
        auto newK = k;
        newK.module = newModule;
        values[newK] = v;
    }

    if (layout != nullptr && layout->count(module)) {
        auto v = layout->at(module);
        layout->erase(module);
        layout->insert_or_assign(newModule, v);
    }

    return 0;
}

int ComponentDescriptor::addPort(const string & portName, bool inputPort, const string & unit, const float & defaultValue) {
    auto& bus = inputPort ? inBus : outBus;
    string actualPortName = portName;
    bool conflict = true;

    // handle conflicts ...
    while (conflict) {
        for (const auto& p : bus) {
            if (p.name == actualPortName) {
                actualPortName += "_";
                conflict = true;
                break;
            }
        }
        conflict = false;
    }

    bus.push_back({ actualPortName, unit, defaultValue });

    return 0;
}

int ComponentDescriptor::renamePort(const string & portName, const string & newPortName, bool inputPort) {
    vector<PadDescription>* bus = nullptr;
    if (inputPort) bus = &inBus;
    else bus = &outBus;

    if (!portNameIsValid(newPortName)) return -2;

    int idx = -1;
    for (int i = 0; i < bus->size(); ++i) {
        auto& pd = bus->at(i);
        if (pd.name == newPortName) return -3;
        else if (pd.name == portName) { idx = i; break; }
    }
    if (idx == -1) return -1;

    (*bus)[idx].name = newPortName;

    for (auto & c : graph.connections) {
        if (inputPort && c.source.port == portName) c.source.port = newPortName;
        else if (!inputPort && c.target.port == portName) c.target.port = newPortName;
    }

    return 0;
}

int ComponentDescriptor::removePort(const string & portName, bool inputPort) {
    int idx = -1;
    int ctr = 0;

    auto& bus = inputPort ? inBus : outBus;
    if (bus.size() <= 1) return -1; // keep at least one or stuff gets stupid

    for (const auto& p : bus) {
        if (p.name == portName) {
            idx = ctr;
            break;
        }
        ctr++;
    }

    if (idx == -1) return -1;

    bus.erase(bus.begin() + idx);

    auto wit = graph.connections.begin();
    while (wit != graph.connections.end()) {
        const auto &src = wit->source;
        const auto &tg = wit->target;
        if (
            (inputPort && src.module == c_inBus.name && src.port == portName) ||
            (!inputPort && tg.module == c_outBus.name && tg.port == portName))
        {
            wit = graph.connections.erase(wit);
        }
        else {
            ++wit;
        }
    }

    return 0;
}

const locale ansi_loc("C");

bool characterIsValid_base(char c) {
    return isupper(c, ansi_loc) || islower(c, ansi_loc) || isdigit(c, ansi_loc) || c == '.' || c == '_';
}

bool characterIsValid(char c, bool isName) {
    bool isValid = characterIsValid_base(c);
    if (isName && !isValid) {
        return c == componentSeparator || c == parameterMarker || c == componentMarker;
    }
    return isValid;
}

bool stringIsValid(string s, bool isName = false) {
    for (auto c : s) {
        if (!characterIsValid(c, isName)) return false;
    }
    return true;
}

bool portNameIsValid(const string& moduleName) {
    return stringIsValid(moduleName, true);
}

bool moduleNameIsValid(const string& moduleName){
    if (moduleName == c_inBus.name || moduleName == c_outBus.name) return false;
    return stringIsValid(moduleName, true);
}

bool moduleTypeIsValid(const string& moduleType) {
    if(moduleType.front() == componentMarker
    || moduleType.front() == parameterMarker
    ) return stringIsValid(moduleType.substr(1));
    return stringIsValid(moduleType);
}

bool componentTypeIsValid(const string& componentType){
    if(componentType.front() != componentMarker) return false;
    return stringIsValid(componentType.substr(1));
}

const vector<PadDescription> c_effectChainInBus = {
    { "left", "", 0 },
    { "right", "", 0 },

    { "mod", "", 0 },
    { "exp", "", 0 },
    { "brt", "", 0 },

    { "sigNom", "", 0 },
    { "sigDen", "", 0 },
    { "barLen", "", 0 },
    { "bpm", "", 0 },
    { "barPos", "", 0 },
    { "pos", "", 0 },
    { "time", "", 0 }

};

const vector<PadDescription> c_effectChainOutBus = {
    { "left", "", 0 },
    { "right", "", 0 },
};

const vector<PadDescription> c_voiceChainInBus = {
    { "gate", "", 0 },
    { "strike_z", "", 0 },
    { "lift_z", "", 0 },
    { "pitch_hz", "", 0 },
    { "glide_x", "", 0 },
    { "slide_y", "", 0 },
    { "press_z", "", 0 },

    { "mod", "", 0 },
    { "exp", "", 0 },
    { "brt", "", 0 },

    { "sigNom", "", 0 },
    { "sigDen", "", 0 },
    { "barLen", "", 0 },
    { "bpm", "", 0 },
    { "barPos", "", 0 },
    { "pos", "", 0 },
    { "time", "", 0 },

    { "noteIndex", "", 0 },
    { "voiceIndex", "", 0 },
    { "polyphony", "" , 0 }
};

const vector<PadDescription> c_voiceChainOutBus = {
    { "left", "", 0 },
    { "right", "", 0 },
};

const ModuleVariable c_inBus = { "inBus", "_INBUS" };
const ModuleVariable c_outBus = { "outBus", "_OUTBUS" };

}
