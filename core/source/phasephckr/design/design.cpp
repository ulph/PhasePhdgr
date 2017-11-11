#include "phasephckr/components.hpp"
#include "phasephckr/design.hpp"

#include "connectiongraph.hpp"
#include <set>
#include <list>
#include <queue>
#include "busmodules.hpp"

#include <assert.h>

using namespace std;

namespace PhasePhckr {

void designChain(
    ConnectionGraph &g,
    const PatchDescriptor &pd,
    ComponentDescriptor &cd,
    map<string, int>& moduleHandles,
    map<string, int>& parameterHandles,
    int depth
);

bool checkModuleName(const string& name){
    if(!moduleNameIsValid(name)){
        cerr << "Error: \"" << name << "\" is not a valid Module name!" << endl;
        return false;
    }
    return true;
}

bool unpackComponent(
    ConnectionGraph &g,
    const ModuleVariable &mv,
    const PatchDescriptor &pd,
    ComponentDescriptor &parent,
    ComponentDescriptor &current,
    map<string, int> &moduleHandles,
    map<string, int> &parameterHandles,
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
        current.graph.values.push_back(ModulePortValue{{c_inBus.name, i.name}, i.value});
    }

    // find any values set on this component and reroute to inBus
    for(auto &v : parent.graph.values){
        if(v.target.module == mv.name){
            v.target.module = inBusName;
        }
    }

    // prefix internals accordingly
    for (auto &m : current.graph.modules) {
        if(!checkModuleName(m.name)) continue;
        m.name = pfx + m.name;
    }
    for (auto &v : current.graph.values) {
        v.target.module = pfx + v.target.module;
    }
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
    map<string, int> &parameterHandles,
    int depth
){

    set<string> components;

    // find any components (module bundles) and unpack them
    for (const auto &m : cd.graph.modules) {
        if (m.type.front() == componentMarker) {
            if(!pd.components.count( m.type )){
                cerr << "Error: \"" << m.type << "\" is unknown Component type!" << endl;
                continue;
            }
            if(!componentNameIsValid(m.type)){
                cerr << "Error: \"" << m.type << "\" is not a valid Component name!" << endl;
                continue;
            }
            ComponentDescriptor child_cd = pd.components.at(m.type);
            if(unpackComponent(g, m, pd, cd, child_cd, moduleHandles, parameterHandles, depth+1)){
                components.insert(m.name);
            }
        }
    }

    // create the modules and store their handles
    for (const auto &m : cd.graph.modules) {
        if(depth == 0 && !checkModuleName(m.name)) continue;
        if (moduleHandles.count(m.name) > 0) {
            cerr << "Error: \"" << m.name << "\" name dupe! (modules)" << endl;
        }
        if (components.count(m.name)) continue;

        int handle = g.addModule(m.type);
        moduleHandles[m.name] = handle;
        cout << handle << " : " << " " << m.name << std::endl;
        if(m.type.front() == parameterMarker){
            parameterHandles[m.name] = handle;
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
    for (const auto &v : cd.graph.values) {
        if (components.count(v.target.module)) continue;
        int h = moduleHandles.count(v.target.module) > 0 ? moduleHandles.at(v.target.module) : -2;
        cout << v.target.module << ":" << v.target.port << " = " << v.value << endl;
        if (h < 0) {
            cerr << "Error: " << v.target.module << " not found! (values)" << endl;
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
    map<string, int> &parameterHandles,
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
}

const string bannedCharacters = " @!?.=-/\"\\"; // etc, quite a list ... make some utility function to build this list instead.

bool findAnyOf(const string& str, const string& chars){
    for(const auto& c: chars){
        if(str.find(c) != string::npos) return true;
    }
    return false;
}

bool moduleNameIsValid(const string& moduleName){
    if(findAnyOf(moduleName, bannedCharacters)) return false;
    return true;
}

bool componentNameIsValid(const string& componentName){
    if(componentName.front() != componentMarker) return false;
    if(findAnyOf(componentName.substr(1), bannedCharacters)) return false;
    return true;
}

const vector<PadDescription> c_effectChainInBus = {
    { "left", "", 0 },
    { "right", "", 0 },

    { "mod", "", 0 },
    { "exp", "", 0 },
    { "brt", "", 0 },

    { "sigNom", "", 0 },
    { "sigDen", "", 0 },
    { "bpm", "", 0 },
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
    { "bpm", "", 0 },
    { "pos", "", 0 },
    { "time", "", 0 }
};

const vector<PadDescription> c_voiceChainOutBus = {
    { "left", "", 0 },
    { "right", "", 0 },
};

const ModuleVariable c_inBus = { "inBus", "_INBUS" };
const ModuleVariable c_outBus = { "outBus", "_OUTBUS" };

}
