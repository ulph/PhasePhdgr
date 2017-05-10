#include "design.hpp"
#include "connectiongraph.hpp"
#include "components.hpp"
#include <set>
#include <list>
#include <queue>
#include "busmodules.hpp"
#include "design.hpp"

using namespace std;

namespace PhasePhckr {

void designChain(
    ConnectionGraph &g,
    PatchDescriptor &pd,
    ComponentDescriptor &cd,
    map<string, int>& handles
);

bool unpackComponent(
    ConnectionGraph &g,
    const ModuleVariable &mv,
    PatchDescriptor &pd,
    ComponentDescriptor &parent,
    ComponentDescriptor &current,
    map<string, int>&handles
) {
    // find the component definition
    cout << "component " << mv.name << " " << mv.type << endl;

    // move component subgraph onto the mv's "scope"
    const string pfx = mv.name + ".";

    auto inBus = new BusModule(current.inBus, true);
    auto outBus = new BusModule(current.outBus, false);
    handles[pfx+c_inBus.name] = g.addCustomModule(inBus);
    handles[pfx+c_outBus.name] = g.addCustomModule(outBus);

    // find any connections to and from this component and "re-route" to inBus/outBus
    for(auto &c : parent.graph.connections){
        if(c.source.module == mv.name){
           c.source.module = pfx+c_outBus.name;
        }
        if(c.target.module == mv.name){
            c.target.module = pfx+c_inBus.name;
        }
    }

    // copy all values from inBus onto values
    for(const auto &i : current.inBus){
        current.graph.values.push_back(ModulePortValue{c_inBus.name, i.name, i.value});
    }

    for (auto &m : current.graph.modules) {
        m.name = pfx + m.name;
    }
    for (auto &v : current.graph.values) {
        v.target.module = pfx + v.target.module;
    }
    for (auto &c : current.graph.connections) {
        c.source.module = pfx + c.source.module;
        c.target.module = pfx + c.target.module;
    }

    designChain(g, pd, current, handles);

    return true;
}


void designChain(
    ConnectionGraph &g,
    PatchDescriptor &pd,
    ComponentDescriptor &cd,
    map<string, int> &handles) {

    set<string> components;

    // find any components (module bundles) and unpack them
    for (const auto &m : cd.graph.modules) {
        if (m.type.front() == '@') {
            if(!pd.components.count( m.type )){
                cerr << "Error: " << m.type << " is unknown Component type!" << endl;
                continue;
            }
            if(unpackComponent(g, m, pd, cd, pd.components[m.type], handles)){
                components.insert(m.name);
            }
        }
    }

    // create the modules and store their handles
    for (const auto &m : cd.graph.modules) {
        if (components.count(m.name)) continue;
        if (handles.count(m.name) > 0) {
            cerr << "Error: " << m.name << " name dupe! (modules)" << endl;
        }
        else {
            int handle = g.addModule(m.type);
            handles[m.name] = handle;
            cout << handle << " : " << " " << m.name << std::endl;
        }
    }

    // iterate over the connections
    for (const auto &c : cd.graph.connections) {
        int hFrom = handles.count(c.source.module) > 0 ? handles.at(c.source.module) : -2;
        int hTo = handles.count(c.target.module) > 0 ? handles.at(c.target.module) : -2;
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
        int h = handles.count(v.target.module) > 0 ? handles.at(v.target.module) : -2;
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
    PatchDescriptor &p,
    const vector<PadDescription>& inBus,
    const vector<PadDescription>& outBus,
    map<string, int> &moduleHandles,
    const ComponentRegister & cp
) {

    // copy over all global components
    for (const auto & kv : cp.all()){
        if(!p.components.count(kv.first)){
            p.components[kv.first] = kv.second;
        }
        else{
            cerr << "Warning: Component " << kv.first << " is globally defined and in patch! Consider renaming the patch defined component." << endl;
        }
    }

    // create the special inBus and outBus modules
    auto inBus_ = new BusModule(inBus, true);
    auto outBus_ = new BusModule(outBus, false);
    moduleHandles[c_inBus.name] = connectionGraph.addCustomModule(inBus_);
    moduleHandles[c_outBus.name] = connectionGraph.addCustomModule(outBus_);

    // begin parsing from root component
    designChain(
        connectionGraph,
        p,
        p.root,
        moduleHandles
    );
}

}
