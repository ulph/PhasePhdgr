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

ConnectionGraph::ConnectionGraph(uint32_t fs) : fs(fs)
{
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
    int id = -1;
    Module *m = nullptr;
    
    if(     !strcmp(type, "PHASE"))  m = new Phase();
    else if(!strcmp(type, "SQUARE")) m = new Square();
    else if(!strcmp(type, "MUL"))    m = new Mul();
    else if(!strcmp(type, "CLAMP"))  m = new Clamp();
    else if(!strcmp(type, "QUANT8")) m = new Quant8();
    else if(!strcmp(type, "NOISE"))  m = new Noise();
    else if(!strcmp(type, "SINE"))   m = new Sine();
    else if(!strcmp(type, "SATAN"))  m = new SaturatorAtan();
    else if(!strcmp(type, "ENV"))    m = new CamelEnvelope();
    else if(!strcmp(type, "LAG"))    m = new Lag();
    else if(!strcmp(type, "RECT"))   m = new Rectifier();
    else if(!strcmp(type, "FOLD"))   m = new FoldBack();
    else if(!strcmp(type, "XFADE"))  m = new CrossFade();
    else if(!strcmp(type, "MULT"))   m = new AddMul();
    
    if(m) {
        id = (int)modules.size();
        modules.push_back(m);
    } else {
        std::cerr << "Error: Module '" << type << "' not found" << std::endl;
    }
    
    return id;
}

int ConnectionGraph::addModule(Module *m){
    int id = -1;
    
    if(m) {
        id = (int)modules.size();
        modules.push_back(m);
    } else {
        std::cerr << "Error: Module invalid" << std::endl;
    }
    
    return id;
}

void ConnectionGraph::connect(int fromModule, int fromPad, int toModule, int toPad)
{
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

void ConnectionGraph::process(int module, uint32_t time)
{
    Module *m = getModule(module);
    if(m->getTime() != time) {
        m->setTime(time);
        
        // Iterate over all input pads
        for(int pad = 0; pad < m->getNumInputPads(); pad++) {
            // Check if other modules are connected to this pad
            for(const Cable *c : cables) {
                if(c->isConnected(module, pad)) {
                    Module *m_dep = getModule(c->getFromModule());
                    // Found connected module
                    process(c->getFromModule(), time);
                    m->setInput(pad, m_dep->getOutput(c->getFromPad())); 
                }
            }
        }
        
        // Process the updated input
        m->doProcess(fs);
    }
}

float ConnectionGraph::getOutput(int module, int pad)
{
    Module *m = getModule(module);
    return m->getOutput(pad);
}
