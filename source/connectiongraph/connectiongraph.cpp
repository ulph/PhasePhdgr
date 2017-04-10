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

ConnectionGraph::ConnectionGraph(uint32_t fs) : fs(fs), compiledForModule(-1)
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
    compiledForModule = -1;
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
    else if(!strcmp(type, "ABS"))    m = new Abs();
    else if(!strcmp(type, "FOLD"))   m = new FoldBack();
    else if(!strcmp(type, "XFADE"))  m = new CrossFade();
    else if(!strcmp(type, "SPOW"))   m = new SymPow();
    else if(!strcmp(type, "CINV"))   m = new ClampInv();
    else if(!strcmp(type, "SCLSHFT"))m = new ScaleShift();
    
    if(m) {
        id = (int)modules.size();
        modules.push_back(m);
    } else {
        std::cerr << "Error: Module '" << type << "' not found" << std::endl;
    }
    
    return id;
}

int ConnectionGraph::addModule(Module *m){
    compiledForModule = -1;
    int id = -1;
    
    if(m) {
        id = (int)modules.size();
        modules.push_back(m);
    } else {
        std::cerr << "Error: Module invalid" << std::endl;
    }
    
    return id;
}

void ConnectionGraph::connect(int fromModule, std::string fromPad, int toModule, std::string toPad)
{
    compiledForModule = -1;
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
    compiledForModule = -1;
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
            float input = 0.0f;
            bool padIsConnected = false;
            // Check if other modules are connected to this pad
            for(const Cable *c : cables) {
                if(c->isConnected(module, pad)) {
                    padIsConnected = true;
                    Module *m_dep = getModule(c->getFromModule());
                    // Found connected module
                    process(c->getFromModule(), time);
                    input += m_dep->getOutput(c->getFromPad());
                }
            }
            
            if(padIsConnected) {
                m->setInput(pad, input); 
            }
        }
    }
    
    // Process the updated input
    m->process(fs);
}

float ConnectionGraph::getOutput(int module, int pad)
{
    Module *m = getModule(module);
    return m->getOutput(pad);
}

void ConnectionGraph::compile(int module)
{
    program.clear();
    for(Module *m : modules) m->setTime(0);
    
    compileInternal(module);
    compiledForModule = module;
}

void ConnectionGraph::compileInternal(int module)
{
    Module *m = getModule(module);
    int time = 1;
    if(m->getTime() != time) {
        m->setTime(time);
        
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
                    compileInternal(c->getFromModule());
                    program.push_back(Instruction(OP_ADD_OUTPUT_TO_INPUT, c->getFromModule(), c->getFromPad(), module, pad));
                }
            }
        }
    }
    
    program.push_back(Instruction(OP_PROCESS, module));
}

void ConnectionGraph::run(int module)
{
    // Recompile if needed
    if(module != compiledForModule) compile(module);
    
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
