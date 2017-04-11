#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>
#include <sstream>
#include "connectiongraph.hpp"
#include "module.hpp"

#define NOT_COMPILED (-1)

class Cable
{
protected:
    int fromModule;
    int fromPad;
    int toModule;
    int toPad;
    
public:
    Cable(int _fromModule, int _fromPad, int _toModule, int _toPad) : fromModule(_fromModule), fromPad(_fromPad), toModule(_toModule), toPad(_toPad) {}
    int getFromModule() const { return fromModule; }
    int getFromPad() const { return fromPad; }
    int getToModule() const { return toModule; }
    bool isConnected(int _toModule, int _toPad) const {
        return toModule == _toModule && toPad == _toPad;
    }
};

ConnectionGraph::ConnectionGraph() : compilationStatus(NOT_COMPILED)
{
}

ConnectionGraph::~ConnectionGraph()
{
    for(Cable *c : cables) {
        delete c;
    }
    
    for(Module *m : modules) {
        delete m;
    }
}

Module* ConnectionGraph::getModule(int id)
{
    if(id >= 0 && id < modules.size()) {
        return modules[id];
    } else {
        std::cerr << "Error: No module with id '" << id << "'" << std::endl; 
    }
    return nullptr;
}

int ConnectionGraph::addModule(const char *type)
{
    compilationStatus = NOT_COMPILED;
    int id = -1;
    Module *m = nullptr;

    for(auto mod : moduleRegister) {
        if(mod.first.compare(type) == 0) {
            m = mod.second();
            break;
        }
    }

    if(m) {
        id = (int)modules.size();
        modules.push_back(m);
        m->setName(type);
    } else {
        std::cerr << "Error: Module '" << type << "' not found" << std::endl;
    }
    
    return id;
}

void ConnectionGraph::registerModule(std::string name, Module* (*moduleFactory)())
{
    moduleRegister.push_back(std::pair<std::string, Module* (*)()>(name, moduleFactory));
}

void ConnectionGraph::connect(int fromModule, std::string fromPad, int toModule, std::string toPad)
{
    compilationStatus = NOT_COMPILED;
    Module *mFrom = getModule(fromModule); 
    Module *mTo = getModule(toModule);
    
    if(mFrom && mTo) {
        int fromPadNo = mFrom->getOutputPadFromName(fromPad);
        int toPadNo = mTo->getInputPadFromName(toPad);
        
        cables.push_back(new Cable(fromModule, fromPadNo, toModule, toPadNo));
    }
}

void ConnectionGraph::connect(int fromModule, int fromPad, int toModule, int toPad)
{
    compilationStatus = NOT_COMPILED;
    cables.push_back(new Cable(fromModule, fromPad, toModule, toPad));
}

void ConnectionGraph::setInput(int module, int pad, float value)
{
    Module *m = nullptr;
    if(module >= 0 && module < modules.size()) {
        m = modules[module];
    }
    
    if(m) {
        m->setInput(pad, value);
    }
}

void ConnectionGraph::setInput(int module, std::string pad, float value){
    if(module >= 0 && module < modules.size()) {
        Module *m = getModule(module);
        if(m){
            setInput(module, m->getOutputPadFromName(pad), value);
        }
    }
}

float ConnectionGraph::getOutput(int module, int pad)
{
    Module *m = getModule(module);
    return m->getOutput(pad);
}

void ConnectionGraph::compileProgram(int module)
{
    program.clear();
    std::vector<int> processedModules;
    
    compileModule(module, processedModules);
    compilationStatus = module;
}

void ConnectionGraph::compileModule(int module, std::vector<int> &processedModules)
{
    Module *m = getModule(module);

    // Check if this module is already processed by the compiler
    for(int processedModule : processedModules) {
        if(module == processedModule) return;
    }
    processedModules.push_back(module);
        
    // Iterate over all input pads
    for(int pad = 0; pad < m->getNumInputPads(); pad++) {
        bool padIsConnected = false;
        // Check if other modules are connected to this pad
        for(const Cable *c : cables) {
            if(c->isConnected(module, pad)) {
                if(!padIsConnected) {
                    padIsConnected = true;
                    program.push_back(Instruction(OP_RESET_INPUT, module, pad));
                }
                Module *m_dep = getModule(c->getFromModule());
                compileModule(c->getFromModule(), processedModules);
                program.push_back(Instruction(OP_ADD_OUTPUT_TO_INPUT, c->getFromModule(), c->getFromPad(), module, pad));
            }
        }
    }
    
    program.push_back(Instruction(OP_PROCESS, module));
}

void ConnectionGraph::process(int module, float fs)
{
    // Recompile if needed
    if(module != compilationStatus) compileProgram(module);
    
    // Run program
    for(const Instruction &i : program) {
        switch(i.opcode) {
        case OP_PROCESS:
            modules[i.param0]->process(fs);
            break;
        case OP_RESET_INPUT:
            modules[i.param0]->setInput(i.param1, 0.0f);
            break;
        case OP_ADD_OUTPUT_TO_INPUT:
            modules[i.param2]->addToInput(i.param3, modules[i.param0]->getOutput(i.param1));
            break;
        }
    }
}

std::string ConnectionGraph::graphviz()
{
    std::stringstream ss;

    ss << "digraph connections {" << std::endl;
    for(const Cable *c : cables) {
        int fromModule = c->getFromModule();
        int toModule = c->getToModule();
        ss << "   \"" << modules[fromModule]->getName() << "(" << fromModule << ")\" -> ";
        ss << "\"" << modules[toModule]->getName() << "(" << toModule << ")\"" << std::endl;
    }
    ss << "}" << std::endl;

    return ss.str();
}
