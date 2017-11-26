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

ConnectionGraph::ConnectionGraph(const ConnectionGraph& other)
    : compilationStatus(NOT_COMPILED)
{
    // copy constructor that creates an non-compiled version
    for(int i=0; i < (int)other.modules.size(); ++i){
        const auto& m = other.modules[i];
        Module * mCopy = m->clone();
        modules.push_back(mCopy);
    }
    for(int i=0; i < (int)other.cables.size(); ++i){
        const auto& c = other.cables[i];
        auto cCopy = new Cable(*c);
        cables.push_back(cCopy);
    }
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
    if(id >= 0 && id < (int)modules.size()) {
        return modules[id];
    } else {
        std::cerr << "Error: No module with id '" << id << "'" << std::endl; 
    }
    return nullptr;
}

int ConnectionGraph::addModule(std::string type)
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

int ConnectionGraph::addCustomModule(Module* module){
    int id = (int)modules.size();
    modules.push_back(module);
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
    modules[module]->setInput(pad, value);
}

void ConnectionGraph::setInput(int module, std::string pad, float value){
    if (module < 0 || module >= (int)modules.size()) return;
    Module *m = getModule(module);
    if(m) m->setInput(m->getInputPadFromName(pad), value);
}

float ConnectionGraph::getOutput(int module, int pad)
{
    Module *m = getModule(module);
    return m->sample_getOutput(pad);
}

void ConnectionGraph::compileProgram(int module)
{
    program.clear();
    std::set<int> processedModules;
    
    compileModule(module, processedModules, std::set<int>());
    compilationStatus = module;
}

void ConnectionGraph::compileModule(int module, std::set<int> &processedModules, std::set<int> processedModulesToHere)
{
    Module *m = getModule(module);

    if (processedModulesToHere.count(module)) hasRecursion = true;
    processedModulesToHere.insert(module);

    // Check if this module is already processed by the compiler
    if (processedModules.count(module)) return;
    processedModules.insert(module);

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
                compileModule(c->getFromModule(), processedModules, processedModulesToHere);
                program.push_back(Instruction(OP_ADD_OUTPUT_TO_INPUT, c->getFromModule(), c->getFromPad(), module, pad));
            }
        }
    }
    
    program.push_back(Instruction(OP_PROCESS, module));
}

void ConnectionGraph::processSample(int module, float fs)
{
    if (module != compilationStatus) compileProgram(module);

    for(const Instruction &i : program) {
        switch(i.opcode) {
        case OP_PROCESS:
            modules[i.param0]->process((uint32_t)fs);
            break;
        case OP_RESET_INPUT:
            modules[i.param0]->sample_resetInput(i.param1);
            break;
        case OP_ADD_OUTPUT_TO_INPUT:
            float out = modules[i.param0]->sample_getOutput(i.param1);
            modules[i.param2]->sample_addToInput(i.param3, out);
            break;
        }
    }
}

void ConnectionGraph::processBlock(int module, float fs, vector<SampleBuffer>& outBuffers) {
    if (module != compilationStatus) compileProgram(module);

    if (hasRecursion) {
        for (uint32_t i = 0; i < k_blockSize; ++i) {
            processSample(module, fs);
            for (int n = 0; n < outBuffers.size(); ++n) {
                outBuffers[n].buf[i] = getOutput(outBuffers[n].module, outBuffers[n].pad);
            }
        }
    }
    else {
        for (const Instruction &i : program) {
            switch (i.opcode) {
            case OP_PROCESS:
                modules[i.param0]->block_process((uint32_t)fs);
                break;
            case OP_RESET_INPUT:
                modules[i.param0]->block_resetInput(i.param1);
                break;
            case OP_ADD_OUTPUT_TO_INPUT:
                float out[ConnectionGraph::k_blockSize] = { 0.f };
                modules[i.param0]->block_getOutput(i.param1, out);
                modules[i.param2]->block_addToInput(i.param3, out);
                break;
            }
        }
        for (int n = 0; n < outBuffers.size(); ++n) {
            modules[outBuffers[n].module]->block_getOutput(outBuffers[n].pad, outBuffers[n].buf);
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

void ConnectionGraph::makeModuleDocs(std::vector<PhasePhckr::ModuleDoc> &docList) {
    for (const auto & p : moduleRegister) {
        auto m = p.second();
        m->setName(p.first);
        docList.emplace_back(m->makeDoc());
        delete m;
    }
}
