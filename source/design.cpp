#include "design.hpp"
#include "connectiongraph.hpp"
#include "components.hpp"
#include <set>
#include <list>
#include <queue>
#include "busmodules.hpp"

using namespace std;

namespace PhasePhckr {

void designConnectionGraph(
    ConnectionGraph &g,
    ConnectionGraphDescriptor& gDesc,
    map<string, int>& handles,
    const ComponentRegister & cp
);

bool unpackComponent(
    ConnectionGraph &g,
    const ModuleVariable &mv, 
    map<string, int>&handles,
    const ComponentRegister & cp
) {
    ComponentDescriptor cD;

    // find the component definition
    cout << "component " << mv.name << " " << mv.type << endl;
    if (!cp.getComponent(mv.type, cD)) {
        cerr << "Error: " << mv.name << " " << mv.type << " component unknown! (modules)" << endl;
        return false;
    }

    // move component subgraph onto the mv's "scope"
    const string pfx = mv.name + ".";

    auto inBus = new BusModule(vector<Pad>(), true);
    auto outBus = new BusModule(vector<Pad>(), false);
    handles[pfx+"inBus"] = g.addCustomModule(inBus);
    handles[pfx+"outBus"] = g.addCustomModule(outBus);

    for (auto &m : cD.graph.modules) {
        m.name = pfx + m.name;
    }
    for (auto &v : cD.graph.values) {
        v.target.module = pfx + v.target.module;
    }
    for (auto &c : cD.graph.connections) {
        c.source.module = pfx + c.source.module;
        c.target.module = pfx + c.target.module;
    }

    // parse the sub graph
    designConnectionGraph(g, cD.graph, handles, cp);

    return true;
}


void designConnectionGraph(
    ConnectionGraph &g,
    ConnectionGraphDescriptor &gDesc,
    map<string, int> &handles,
    const ComponentRegister &cp) {

    set<string> components;

    // find any components (module bundles) and unpack them
    for (const auto &m : gDesc.modules) {
        if (m.type.front() == '@') {
            if(unpackComponent(g, m, handles, cp)){
                components.insert(m.name);
            }
        }
    }

    // create the modules and store their handles
    for (const auto &m : gDesc.modules) {
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
    for (const auto &c : gDesc.connections) {
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
    for (const auto &v : gDesc.values) {
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

void designConnectionGraph(
    ConnectionGraph &connectionGraph,
    PatchDescriptor &description,
    map<string, int> &moduleHandles,
    const ComponentRegister & cpGlobal
) {
    ComponentRegister cp = cpGlobal;
    for (const auto & c : description.components) {
        cp.registerComponent(c.first, c.second);
    }
    designConnectionGraph(
        connectionGraph, 
        description.root.graph,
        moduleHandles, 
        cp
    );
}

}
