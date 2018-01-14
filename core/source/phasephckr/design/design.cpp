#include "phasephckr/components.hpp"
#include "phasephckr/design.hpp"

#include "connectiongraph.hpp"
#include <set>
#include <list>
#include <queue>
#include <locale>

#include <regex>

#include "busmodules.hpp"

#include <assert.h>

using namespace std;

namespace PhasePhckr {

void designChain(
    ConnectionGraph &g,
    const PatchDescriptor &pd,
    ComponentDescriptor &cd,
    map<string, int> &moduleHandles,
    ParameterHandleMap &parameterHandles,
    int depth
);

bool checkName(const string& name) {
    if (!nameIsValid(name, true)) {
        cerr << "Error: \"" << name << "\" is not a valid Module name!" << endl;
        return false;
    }
    return true;
}

bool checkModule(const ModuleVariable& m){
    if(!genericTypeIsValid(m.type, true)){
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
    if (!componentTypeIsValid(m.type, true)) {
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
    ParameterHandleMap &parameterHandles,
    int depth
) {

    assert(depth > 0);

    // find the component definition
    cout << "component " << mv.name << " " << mv.type << endl;

    // move component subgraph onto the mv's "scope"
    const string pfx = mv.name + scopeSeparator;

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

    designChain(g, pd, current, moduleHandles, parameterHandles, depth);

    return true;
}


void designChain(
    ConnectionGraph &g,
    const PatchDescriptor &pd,
    ComponentDescriptor &cd,
    map<string, int> &moduleHandles,
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
            if(unpackComponent(g, m, pd, cd, child_cd, moduleHandles, parameterHandles, depth+1)){
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
            prm.v.min = 0.f;
            prm.v.max = 1.f;
            prm.v.val = 0.f;
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
            if (v.target.port == "value")   parameterHandles[h].v.val = v.value;
            else if(v.target.port == "min") parameterHandles[h].v.min = v.value;
            else if(v.target.port == "max") parameterHandles[h].v.max = v.value;
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
    for (const auto& c : components) {
        for (const auto& kv : c.second.graph.modules) {
            const ModuleVariable m(kv);
            usedTypes.insert(m.type);
        }
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

void ComponentDescriptor::componentTypeWasRenamed(const string& type, const string& newType) {
    for (auto& kv : graph.modules) {
        if (kv.second == type) kv.second = newType;
    }
}

void ComponentDescriptor::componentTypePortWasRenamed(const string& type, const string& port, const string& newPort, bool inputPort) {
    set<string> instances;
    for (const auto& kv : graph.modules) {
        if (kv.second == type) instances.insert(kv.first);
    }

    for (const auto& m : instances) {
        auto mp = ModulePort(m, port);
        auto newMp = ModulePort(m, newPort);
        for (auto & c : graph.connections) {
            if (inputPort && c.target == mp) c.target = newMp;
            else if (!inputPort && c.source == mp) c.source = newMp;
        }
        if (inputPort && graph.values.count(mp)) {
            auto v = graph.values.at(mp);
            graph.values.erase(mp);
            graph.values[newMp] = v;
        }
    }
}

int PatchDescriptor::renameComponentTypePort(const string& type, const string& port, const string& newPort, bool inputPort) {
    if (!components.count(type)) return -4;
    auto ret = components[type].renamePort(port, newPort, inputPort);
    if (ret != 0) return ret;

    root.componentTypePortWasRenamed(type, port, newPort, inputPort);
    for (auto& c : components) {
        c.second.componentTypePortWasRenamed(type, port, newPort, inputPort);
    }

    return 0;
}

int PatchDescriptor::renameComponentType(const string& type, const string& newType) {
    if (!components.count(type)) return -1;
    if (!componentTypeIsValid(newType, false)) return -2;
    if (components.count(newType)) return -3;
    auto v = components[type];
    components.erase(type);
    components[newType] = v;

    root.componentTypeWasRenamed(type, newType);
    for (auto& c : components) {
        c.second.componentTypeWasRenamed(type, newType);
    }

    return 0;
}

int PatchDescriptor::addComponentType(string& type, const ComponentDescriptor& descriptor, bool resolveNameConflict) {
    if (!componentTypeIsValid(type, true)) return -1;
    if (!resolveNameConflict && components.count(type)) return -2;

    string newType = type;
    while (components.count(newType)) newType += "_";

    type = newType;
    components[type] = descriptor;
    return 0;
}

int PatchDescriptor::createNewComponentType(ComponentDescriptor* rootComponent, const set<string>& modules, string& newType) {
    if (modules.size() < 2) return -1;
    if (rootComponent == nullptr) return -2;

    string t = string(&componentMarker) + "CMP";
    ComponentDescriptor cd;

    int i = 0;
    while (components.count(t + to_string(i))) {
        i++;
    }
    t += to_string(i);

    if (!componentTypeIsValid(t, false)) return -2;

    newType = t;

    i = 0;
    auto m = "new_" + t.substr(1);
    while (rootComponent->graph.modules.count(m + "_" + to_string(i))) i++;
    m += "_" + to_string(i);

    set<string> outBusAlias;
    set<string> inBusAlias;
    map<ModulePort, string> externalSourcesAlias;
    map<ModulePort, string> externalTargetsAlias;

    auto cit = rootComponent->graph.connections.begin();
    while (cit != rootComponent->graph.connections.end()) {
        auto& c = *cit;
        if (modules.count(c.source.module) && modules.count(c.target.module)) {
            cd.graph.connections.push_back(c);
            cit = rootComponent->graph.connections.erase(cit);
        }
        else if (modules.count(c.source.module)) {
            bool prune = true;
            if (!externalTargetsAlias.count(c.target)) {
                auto p_ = c.target.port;
                while (outBusAlias.count(p_)) p_ += "_";
                outBusAlias.insert(p_);
                externalTargetsAlias[c.target] = p_;
                cd.outBus.push_back(PadDescription(p_, "", 0.0f));
                prune = false;
            }
            auto p = externalTargetsAlias.at(c.target);
            cd.graph.connections.push_back(ModulePortConnection(c.source, ModulePort(c_outBus.name, p)));
            if (prune) {
                cit = rootComponent->graph.connections.erase(cit);
            }
            else {
                c.source.module = m;
                c.source.port = p;
                ++cit;
            }
        }
        else if (modules.count(c.target.module)) {
            bool prune = true;
            if (!externalSourcesAlias.count(c.source)) {
                auto p_ = c.source.port;
                while (inBusAlias.count(p_)) p_ += "_";
                inBusAlias.insert(p_);
                externalSourcesAlias[c.source] = p_;
                auto v = 0.0f;
                if (rootComponent->graph.values.count(c.target)) { v = rootComponent->graph.values.at(c.target); }
                cd.inBus.push_back(PadDescription(p_, "", v));
                prune = false;
            }
            auto p = externalSourcesAlias.at(c.source);
            cd.graph.connections.push_back(ModulePortConnection(ModulePort(c_inBus.name, p), c.target));
            if (prune) {
                cit = rootComponent->graph.connections.erase(cit);
            }
            else {
                c.target.module = m;
                c.target.port = p;
                ++cit;
            }
        }
        else {
            ++cit;
        }
    }

    auto mit = rootComponent->graph.modules.begin();
    while (mit != rootComponent->graph.modules.end()) {
        if (modules.count(mit->first)) {
            cd.graph.modules[mit->first] = mit->second;
            mit = rootComponent->graph.modules.erase(mit);
        }
        else ++mit;
    }

    auto vit = rootComponent->graph.values.begin();
    while (vit != rootComponent->graph.values.end()) {
        if (modules.count(vit->first.module)) {
            cd.graph.values[vit->first] = vit->second;
            vit = rootComponent->graph.values.erase(vit);
        }
        else ++vit;
    }

    components[newType] = cd;
    rootComponent->graph.modules[m] = newType;

    return 0;
}

int PatchDescriptor::removeComponentType(const string& type) {
    if (!components.count(type)) return -1;
    components.erase(type);
    return 0;
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

void ConnectionGraphDescriptor::pruneOrphanValues() {
    auto it = values.begin();
    while (it != values.end()) {
        if (!modules.count(it->first.module)) it = values.erase(it);
        else ++it;
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
                break;
            }
        }
        if (!prunedStuff) ++it;
    }
}

int ConnectionGraphDescriptor::add(const string& module, const string& type) {
    if (!nameIsValid(module, false)) return -1;
    if (!genericTypeIsValid(type, true)) return -2;
    if (modules.count(module)) return -3;
    modules[module] = type;
    return 0;
}

int ConnectionGraphDescriptor::remove(const string& module) {
    if (!modules.count(module)) return -1;
    modules.erase(module);
    pruneDanglingConnections();
    pruneOrphanValues();
    return 0;
}

int ConnectionGraphDescriptor::rename(const string& module, const string& newModule) {
    if (!modules.count(module)) return -1;
    if (modules.count(newModule)) return -2;
    if (!nameIsValid(newModule, false)) return -3;

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
            conflict = false;
        }
    }

    bus.push_back({ actualPortName, unit, defaultValue });

    return 0;
}

int ComponentDescriptor::changePortUnit(const string & portName, const string & newUnit) {
    for (auto& pd : inBus) {
        if (pd.name == portName) {
            pd.unit = newUnit;
            return 0;
        }
    }
    return -1;
}

int ComponentDescriptor::changePortValue(const string & portName, float newValue) {
    for (auto& pd : inBus) {
        if (pd.name == portName) {
            pd.defaultValue = newValue;
            return 0;
        }
    }
    return -1;
}

bool ComponentDescriptor::hasPort(const string & portName, bool inputPort) {
    auto& bus = inputPort ? inBus : outBus;
    for (auto& pd : bus) {
        if (pd.name == portName) {
            return true;
        }
    }
    return false;
}

int ComponentDescriptor::getPort(const string & portName, PadDescription& result, bool inputPort) {
    auto& bus = inputPort ? inBus : outBus;
    for (auto& pd : bus) {
        if (pd.name == portName) {
            result = pd;
            return 0;
        }
    }
    return -1;
}

int ComponentDescriptor::renamePort(const string & portName, const string & newPortName, bool inputPort) {
    vector<PadDescription>* bus = nullptr;
    if (inputPort) bus = &inBus;
    else bus = &outBus;

    if (!portIsValid(newPortName)) return -2;

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

int ComponentDescriptor::addModule(const string& type, string& name) {
    if (!genericTypeIsValid(type, true)) return -1;

    string newName = nameFromType(type);

    int i = 0;
    string fullName = newName + to_string(i);
    while (graph.add(fullName, type)) {
        if (!nameIsValid(fullName, false)) return -2;
        i++;
        fullName = newName + to_string(i);
    }

    name = fullName;
    return 0;
}

NoteStealPolicy PresetSettings::getNoteStealPolicy() {
    if (noteStealPolicy == NoteStealPolicyAuto) {
        if (polyphony <= 4) return NoteStealPolicyClosest;
        return NoteStealPolicyNone;
    }
    return noteStealPolicy;
}

NoteReactivationPolicy PresetSettings::getNoteReactivationPolicy() {
    if (noteReactivationPolicy == NoteReactivationPolicyAuto) {
        if (polyphony <= 4)
        {
            switch (getNoteStealPolicy()) {
            case NoteStealPolicyOldest:
                return NoteReactivationPolicyLast;
            case NoteStealPolicyYoungest:
                return NoteReactivationPolicyLast;
            case NoteStealPolicyIfLower:
                return NoteReactivationPolicyClosest;
            case NoteStealPolicyIfHigher:
                return NoteReactivationPolicyClosest;
            case NoteStealPolicyClosest:
                return NoteReactivationPolicyClosest;
            default: break;
            }
        }
        return NoteReactivationPolicyNone;
    }
    return noteReactivationPolicy;
}


const locale ansi_loc("C");

bool characterIsValid(char c, bool allowLower, bool allowScope) {
    bool isValid = isupper(c, ansi_loc) || isdigit(c, ansi_loc) || c == '_';
    if (!isValid && allowLower) {
        isValid = islower(c, ansi_loc);
    }
    if (!isValid && allowScope) {
        isValid = c == scopeSeparator;
    }
    return isValid;
}

// ...

bool portIsValid(const string& s) {
    for (auto c : s) {
        if (!characterIsValid(c, true, false)) return false;
    }
    return true;
}

bool nameIsValid(const string& s, bool allowScope){
    if (s == c_inBus.name || s == c_outBus.name) return false;
    for (auto c : s) {
        if (!characterIsValid(c, true, allowScope)) return false;
    }
    return true;
}

bool typeIsValid(const string& s, bool allowScope) {
    if (s == c_inBus.type || s == c_outBus.type) return false;
    for (auto c : s) {
        if (!characterIsValid(c, true, allowScope)) return false;
    }
    return true;
}

bool componentTypeIsValid(const string& s, bool allowScope) {
    if (s.front() != componentMarker) return false;
    return typeIsValid(s.substr(1), allowScope);
}

bool parameterTypeIsValid(const string& s, bool allowScope) {
    if (s.front() != componentMarker) return false;
    return typeIsValid(s.substr(1), allowScope);
}

bool genericTypeIsValid(const string& s, bool allowScope) {
    if (s.front() == componentMarker) return typeIsValid(s.substr(1), allowScope);
    else if(s.front() == parameterMarker) return typeIsValid(s.substr(1), allowScope);
    return typeIsValid(s, allowScope);
}

// ...

string nameFromType(const string& type) {
    string name = type;

    auto re = regex("\\" + string(1, scopeSeparator)); // remove any paths
    name = regex_replace(name, re, "_");

    if (name.front() == componentMarker) {
        name = name.substr(1);
    }
    else if (name.front() == parameterMarker) {
        name = name.substr(1);
    }

    name = "new_" + name + "_";

    return name;
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
    { "noteIndex2", "", 0 },
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
