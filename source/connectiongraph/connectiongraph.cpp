#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>
#include "connectiongraph.hpp"
#include "module.hpp"
#include "inputoutputbus.hpp"

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
    moduleRegister.push_back(std::pair<std::string, Module* (*)()>(std::string("INPUT"), &(InputBus::factory)));
    moduleRegister.push_back(std::pair<std::string, Module* (*)()>(std::string("OUTPUT"), &(OutputBus::factory)));
    moduleRegister.push_back(std::pair<std::string, Module* (*)()>(std::string("PHASE"), &(Phase::factory)));
    moduleRegister.push_back(std::pair<std::string, Module* (*)()>(std::string("SQUARE"), &(Square::factory)));
    moduleRegister.push_back(std::pair<std::string, Module* (*)()>(std::string("MUL"), &(Mul::factory)));
    moduleRegister.push_back(std::pair<std::string, Module* (*)()>(std::string("CLAMP"), &(Clamp::factory)));
    moduleRegister.push_back(std::pair<std::string, Module* (*)()>(std::string("QUANT8"), &(Quant8::factory)));
    moduleRegister.push_back(std::pair<std::string, Module* (*)()>(std::string("NOISE"), &(Noise::factory)));
    moduleRegister.push_back(std::pair<std::string, Module* (*)()>(std::string("SINE"), &(Sine::factory)));
    moduleRegister.push_back(std::pair<std::string, Module* (*)()>(std::string("SATAN"), &(SaturatorAtan::factory)));
    moduleRegister.push_back(std::pair<std::string, Module* (*)()>(std::string("ENV"), &(CamelEnvelope::factory)));
    moduleRegister.push_back(std::pair<std::string, Module* (*)()>(std::string("LAG"), &(Lag::factory)));
    moduleRegister.push_back(std::pair<std::string, Module* (*)()>(std::string("ABS"), &(Abs::factory)));
    moduleRegister.push_back(std::pair<std::string, Module* (*)()>(std::string("FOLD"), &(FoldBack::factory)));
    moduleRegister.push_back(std::pair<std::string, Module* (*)()>(std::string("XFADE"), &(CrossFade::factory)));
    moduleRegister.push_back(std::pair<std::string, Module* (*)()>(std::string("SPOW"), &(SymPow::factory)));
    moduleRegister.push_back(std::pair<std::string, Module* (*)()>(std::string("CINV"), &(ClampInv::factory)));
    moduleRegister.push_back(std::pair<std::string, Module* (*)()>(std::string("SCLSHFT"), &(ScaleShift::factory)));
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

    for(auto mod : moduleRegister) {
        if(mod.first.compare(type) == 0) {
            m = mod.second();
        }
    }

    if(m) {
        id = (int)modules.size();
        modules.push_back(m);
    } else {
        std::cerr << "Error: Module '" << type << "' not found" << std::endl;
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

float ConnectionGraph::getOutput(int module, int pad)
{
    Module *m = getModule(module);
    return m->getOutput(pad);
}

void ConnectionGraph::compileProgram(int module)
{
    program.clear();
    for(Module *m : modules) m->setProcessed(false);
    
    compileModule(module);
    compiledForModule = module;
}

void ConnectionGraph::compileModule(int module)
{
    Module *m = getModule(module);
    if(!m->isProcessed()) {
        m->setProcessed(true);
        
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
                    compileModule(c->getFromModule());
                    program.push_back(Instruction(OP_ADD_OUTPUT_TO_INPUT, c->getFromModule(), c->getFromPad(), module, pad));
                }
            }
        }
    }
    
    program.push_back(Instruction(OP_PROCESS, module));
}

void ConnectionGraph::process(int module)
{
    // Recompile if needed
    if(module != compiledForModule) compileProgram(module);
    
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
