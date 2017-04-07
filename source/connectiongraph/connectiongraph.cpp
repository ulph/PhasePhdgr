#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>
#include "module.hpp"
#include "connectiongraph.hpp"

class Cable
{
protected:
    int fromModule;
    int toModule;
    int toPad;
    
public:
    Cable(int _fromModule, int _toModule, int _toPad) : fromModule(_fromModule), toModule(_toModule), toPad(_toPad) {}
    int getFromModule() const { return fromModule; }
    int getToModule() const { return toModule; }
    bool isConnected(int _toModule, int _toPad) const {
        return toModule == _toModule && toPad == _toPad;
    }
};

ConnectionGraph::ConnectionGraph(uint32_t fs) : fs(fs)
{
    std::cerr << Phase::desc
              << Square::desc
              << Add::desc
              << Mul::desc
              << Clamp::desc
              << Quant8::desc;
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
    Module *m = nullptr;
    int id = -1;
    
    if(     !strcmp(type, "PHASE"))  m = new Phase();
    else if(!strcmp(type, "SQUARE")) m = new Square();
    else if(!strcmp(type, "ADD"))    m = new Add();
    else if(!strcmp(type, "MUL"))    m = new Mul();
    else if(!strcmp(type, "CLAMP"))  m = new Clamp();
    else if(!strcmp(type, "QUANT8"))  m = new Quant8();
    
    if(m) {
        id = (int)modules.size();
        modules.push_back(m);
    } else {
        std::cerr << "Error: Module '" << type << "' not found" << std::endl;
    }
    
    return id;
}

void ConnectionGraph::connect(int fromModule, int toModule, int toPad)
{
    cables.push_back(new Cable(fromModule, toModule, toPad));
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

float ConnectionGraph::getOutput(int module, float time)
{
    Module *m = getModule(module);
    if(m->getTime() != time) {
        m->setTime(time);
        
        // Iterate over all input pads
        for(int pad = 0; pad < m->getNumInputPads(); pad++) {
            // Check if other modules are connected to this pad
            for(const Cable *c : cables) {
                if(c->isConnected(module, pad)) {
                    // Found connected module
                    m->setInput(pad, getOutput(c->getFromModule(), time)); 
                }
            }
        }
        
        // Process the updated input
        m->process(fs);
    }
    
    return m->getOutput();
}

