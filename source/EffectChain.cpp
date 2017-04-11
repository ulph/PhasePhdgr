#include "EffectChain.hpp"
#include "BusModules.hpp"
#include "moduleregister.hpp"
#include <utility>
#include <vector>
#include <map>

namespace PhasePhckr {

struct ModuleVariable {
    const char * name;
    const char * type;
};

struct ModulePortConnection {
    const char * fromModule;
    const char * fromPort;
    const char * toModule;
    const char * toPort;
};

struct ModulePortValue {
    const char * module;
    const char * port;
    float value;
};

struct BusHandles {
    int inBus;
    int outBus;
    BusHandles(int inBus, int outBus) : inBus(inBus), outBus(outBus){}
};

BusHandles DesignConnectionGraph(
    ConnectionGraph & connectionGraph,
    ModuleVariable inputBusVariable,
    ModuleVariable outputBusVariable,
    const std::vector<ModuleVariable> &modules,
    const std::vector<ModulePortConnection> &connections,
    const std::vector<ModulePortValue> defaultValues
){
    // internal place to bounce strings to handles
    std::map<const char*, int> moduleHandles;

    // output - outside wants the handles for these two special modules
    BusHandles bus(
        connectionGraph.addModule(inputBusVariable.type),
        connectionGraph.addModule(outputBusVariable.type)
    );
    moduleHandles[inputBusVariable.name] = bus.inBus;
    moduleHandles[outputBusVariable.name] = bus.outBus;

    // create the modules and store their handles
    for(const auto &m:modules){
        if(moduleHandles.count(m.name) > 0){
            // some error printout
        }
        else{
            moduleHandles[m.name] = connectionGraph.addModule(m.type);
        }
    }

    // iterate over the connections
    for(const auto &c:connections){
        int fromModuleHandle = moduleHandles.count(c.fromModule) > 0 ? moduleHandles.at(c.fromModule) : -2;
        int toModuleHandle = moduleHandles.count(c.toModule) > 0 ? moduleHandles.at(c.toModule) : -2;
        if( fromModuleHandle >= 0 && toModuleHandle >= 0 ){
            connectionGraph.connect(fromModuleHandle, c.fromPort, toModuleHandle, c.toPort);
        }
        else {
            // some error printout
        }
    }

    // set default values provided
    for(const auto &v:defaultValues){
        int targetHandle = moduleHandles.count(v.module) > 0 ? moduleHandles.at(v.module) : -2;
        if( targetHandle >= 0) {
            connectionGraph.setInput(targetHandle, v.port, v.value);
        }
        else {
            // some error printout
        }
    }

    return bus;
}

EffectChain::EffectChain(){
    connectionGraph.registerModule("STEREOBUS", &StereoBus::factory);
    ModuleRegister::registerAllModules(connectionGraph);

    BusHandles bus = DesignConnectionGraph(
        connectionGraph,
        ModuleVariable{"inBus", "STEREOBUS"},
        ModuleVariable{"outBus", "STEREOBUS"},
        std::vector<ModuleVariable>(),
        std::vector<ModulePortConnection>{
            ModulePortConnection{"inBus", "Left", "outBus", "Left"},
            ModulePortConnection{"inBus", "Right", "outBus", "Right"},
        },
        std::vector<ModulePortValue>()
    );

    inBus = bus.inBus;
    outBus = bus.outBus;
}

void EffectChain::update(float * bufferL, float * bufferR, int numSamples, float sampleRate, const GlobalData& globalData){
    for (int i = 0; i < numSamples; ++i) {
        connectionGraph.setInput(inBus, 0, bufferL[i]);
        connectionGraph.setInput(inBus, 1, bufferR[i]);
        connectionGraph.setInput(inBus, 2, globalData.mod);
        connectionGraph.setInput(inBus, 3, globalData.exp);
        connectionGraph.setInput(inBus, 4, globalData.brt);

        connectionGraph.process(outBus, sampleRate);

        float sampleL = connectionGraph.getOutput(outBus, 0);
        float sampleR = connectionGraph.getOutput(outBus, 1);
        bufferL[i] = sampleL;
        bufferR[i] = sampleR;
    }
}

}
