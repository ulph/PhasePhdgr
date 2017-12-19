#include <iostream>
#include <vector>
#include <algorithm> 
#include <cmath>
#include <cstring>
#include <sstream>
#include "connectiongraph.hpp"
#include "module.hpp"
#include <assert.h>

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
    bool isConnectedFrom(int _fromModule, int _fromPad) const {
        return fromModule == _fromModule && fromPad == _fromPad;
    }

};

ConnectionGraph::ConnectionGraph(bool forceSampleWise) 
    : compilationStatus(NOT_COMPILED)
    , forceSampleWise(forceSampleWise)
{

}

ConnectionGraph::ConnectionGraph(const ConnectionGraph& other)
    : compilationStatus(other.compilationStatus)
    , forceSampleWise(other.forceSampleWise)
    , program(other.program)
    , fsCompiled(other.fsCompiled)
{
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

void ConnectionGraph::setInputBlock(int module, int pad, const float* value)
{
    modules[module]->block_setInput(pad, value);
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

void ConnectionGraph::getOutputBlock(int module, int pad, float* buffer)
{
    modules[module]->block_getOutput(pad, buffer);
}

void ConnectionGraph::compileProgram(int module, float fs)
{
    if (fs == -1) return;

    fsCompiled = fs;

    // parse the graph once to find all modules involved in recursion loops
    recursiveModules.clear();
    recursiveScannedModules.clear();

    if (!forceSampleWise) {
        findRecursions(module, std::vector<int>());

        // sort cables so that the ones that switch BlockWise to SampleWise and vice versa gets processed first
        std::partition(
            cables.begin(),
            cables.end(),
            [this](const Cable *a) {
                auto aFromType = getProcessingType(a->getFromModule());
                auto aToType = getProcessingType(a->getToModule());
                return aFromType != aToType;
            }
        );
    }

    program.clear();
    std::vector<Instruction> protoProgram;
    std::set<int> processedModules;
    std::set<int> visitedModules;
    compileModule(protoProgram, module, processedModules, visitedModules);
    compilationStatus = module;

    if (!forceSampleWise) {
        finalizeProgram(protoProgram);
    }
    else {
        program = protoProgram;
    }
}

void ConnectionGraph::finalizeProgram(std::vector<Instruction>& protoProgram) {
    assert(!forceSampleWise);
    assert(program.size() == 0);

    std::map<int, std::set<int>> sampleWiseEntrypoints;

    for (int i = 0; i < protoProgram.size(); ++i) {
        auto instr = protoProgram.at(i);
        auto type = getProcessingType(instr.param0);
        auto toType = getProcessingType(instr.param2);

        if (type == SampleWise && (
                instr.opcode == OP_PROCESS 
                ||
                (instr.opcode == OP_SET_OUTPUT_TO_INPUT && toType == SampleWise)
                ||
                (instr.opcode == OP_ADD_OUTPUT_TO_INPUT && toType == SampleWise)
            )
        ){
            std::vector<Instruction> segment;
            while (type == SampleWise) {
                instr = protoProgram.at(i);
                type = getProcessingType(instr.param0);
                if (type == BlockWise) break;
                segment.push_back(instr);
                i++;
            }

            std::vector<Instruction> expandedSegment;
            std::map<int, Instruction> postLoopInstructions;

            for (int n = 0; n < k_blockSize; ++n) {
                for (int j = 0; j < segment.size(); ++j) {
                    auto instr_ = segment.at(j);
                    switch (instr_.opcode) {
                    case OP_PROCESS:
                        if (sampleWiseEntrypoints.count(instr_.param0)) {
                            for (int port : sampleWiseEntrypoints.at(instr_.param0)) {
                                expandedSegment.push_back(Instruction(OP_X_UNBUFFER_ADD_INPUT, instr_.param0, port, n));
                            }
                        }
                        expandedSegment.push_back(instr_);
                        if (sampleWiseEntrypoints.count(instr_.param0)) {
                            for (int port : sampleWiseEntrypoints.at(instr_.param0)) {
                                expandedSegment.push_back(Instruction(OP_CLEAR_INPUT, instr_.param0, port));
                            }
                        }
                        break;
                    case OP_SET_OUTPUT_TO_INPUT:
                        if (getProcessingType(instr_.param2) == BlockWise) {
                            expandedSegment.push_back(Instruction(OP_X_BUFFER_CLEAR_OUTPUT, instr_.param0, instr_.param1, n));
                        }
                    case OP_ADD_OUTPUT_TO_INPUT:
                        if (getProcessingType(instr_.param2) == BlockWise) {
                            if (!postLoopInstructions.count(j)) {
                                auto instr__ = instr_;
                                instr__.opcode = instr__.opcode == OP_SET_OUTPUT_TO_INPUT ? OP_B_SET_OUTPUT_TO_INPUT : OP_B_ADD_OUTPUT_TO_INPUT;
                                postLoopInstructions[j] = instr__;
                            }
                            expandedSegment.push_back(Instruction(OP_X_BUFFER_ADD_OUTPUT, instr_.param0, instr_.param1, n));
                        }
                        else {
                            expandedSegment.push_back(instr_);
                        }
                        break;
                    default:
                        assert(0);
                        break;
                    }
                }
            }

            program.insert(program.end(), expandedSegment.begin(), expandedSegment.end());
            for (const auto& kv : postLoopInstructions)
                program.push_back(kv.second);

        }

        if (type == BlockWise) 
        {
            switch (instr.opcode) {
            case OP_PROCESS:
                instr.opcode = OP_B_PROCESS; break;
            case OP_SET_OUTPUT_TO_INPUT:
                instr.opcode = OP_B_SET_OUTPUT_TO_INPUT; 
                break;
            case OP_ADD_OUTPUT_TO_INPUT:
                instr.opcode = OP_B_ADD_OUTPUT_TO_INPUT; 
                break;
            default:
                assert(0);
                break;
            }
            program.emplace_back(instr);
            if (getProcessingType(instr.param2) == SampleWise && instr.opcode == OP_B_SET_OUTPUT_TO_INPUT || instr.opcode == OP_B_ADD_OUTPUT_TO_INPUT) {
                if (!sampleWiseEntrypoints.count(instr.param2)) sampleWiseEntrypoints[instr.param2] = std::set<int>();
                sampleWiseEntrypoints[instr.param2].insert(instr.param3);
                if (instr.opcode == OP_B_SET_OUTPUT_TO_INPUT) {
                    program.emplace_back(Instruction(OP_CLEAR_INPUT, instr.param2, instr.param3));
                }
            }
        }
        else {
            assert(0);
        }
    }
}

void ConnectionGraph::printProgram(const vector<Instruction>& p) {
    // TODO - not threadsafe
    for (int i = 0; i < p.size(); ++i) {
        const auto& instr = p[i];
        std::cout << i << ": ";
        switch (instr.opcode) {

        case OP_PROCESS:
            std::cout << "OP_PROCESS " << instr.param0; 
            break;
        case OP_CLEAR_INPUT:
            std::cout << "OP_CLEAR_INPUT " << instr.param0 << "," << instr.param1;
            break;
        case OP_SET_OUTPUT_TO_INPUT:
            std::cout << "OP_SET_OUTPUT_TO_INPUT " << instr.param0 << "," << instr.param1 << " -> " << instr.param2 << "," << instr.param3;
            break;
        case OP_ADD_OUTPUT_TO_INPUT:
            std::cout << "OP_ADD_OUTPUT_TO_INPUT " << instr.param0 << "," << instr.param1 << " -> " << instr.param2 << "," << instr.param3; 
            break;

        case OP_X_UNBUFFER_CLEAR_INPUT:
            std::cout << "OP_X_UNBUFFER_CLEAR_INPUT " << instr.param2 << " " << instr.param0 << "," << instr.param1;
            break;
        case OP_X_BUFFER_CLEAR_OUTPUT:
            std::cout << "OP_X_BUFFER_CLEAR_OUTPUT " << instr.param2 << " " << instr.param0 << "," << instr.param1;
            break;

        case OP_X_UNBUFFER_SET_INPUT:
            std::cout << "OP_X_UNBUFFER_SET_INPUT " << instr.param2 << " " << instr.param0 << "," << instr.param1;
            break;
        case OP_X_BUFFER_SET_OUTPUT:
            std::cout << "OP_X_BUFFER_SET_OUTPUT " << instr.param2 << " " << instr.param0 << "," << instr.param1;
            break;

        case OP_X_UNBUFFER_ADD_INPUT:
            std::cout << "OP_X_UNBUFFER_INPUT " << instr.param2 << " " << instr.param0 << "," << instr.param1;
            break;
        case OP_X_BUFFER_ADD_OUTPUT:
            std::cout << "OP_X_BUFFER_OUTPUT " << instr.param2 << " " << instr.param0 << "," << instr.param1;
            break;

        case OP_B_PROCESS:
            std::cout << "OP_B_PROCESS " << instr.param0;
            break;
        case OP_B_SET_OUTPUT_TO_INPUT:
            std::cout << "OP_B_SET_OUTPUT_TO_INPUT " << instr.param0 << "," << instr.param1 << " -> " << instr.param2 << "," << instr.param3;
            break;
        case OP_B_ADD_OUTPUT_TO_INPUT:
            std::cout << "OP_B_ADD_OUTPUT_TO_INPUT " << instr.param0 << "," << instr.param1 << " -> " << instr.param2 << "," << instr.param3;
            break;

        default:
            std::cout << "? " << instr.opcode;
            break;
        }
        std::cout << std::endl;
    }
}

void ConnectionGraph::findRecursions(int module, std::vector<int> processedModulesToHere) {
    bool doneDone = true;
    bool foundSelf = false;
    for (auto otherModule : processedModulesToHere) {
        doneDone = doneDone && recursiveScannedModules.count(otherModule);
        if (otherModule == module) foundSelf = true;
        if (foundSelf) {
            recursiveModules.insert(otherModule);
        }
    }
    if (foundSelf) return;
    processedModulesToHere.push_back(module);

    Module *m = getModule(module);
    for (int pad = 0; pad < m->getNumInputPads(); pad++) {
        for (const Cable *c : cables) {
            if (c->isConnected(module, pad)) {
                auto from = c->getFromModule();
                if (doneDone && recursiveScannedModules.count(from)) {
                    recursivePathsSkipped++;
                    continue;
                }
                findRecursions(from, processedModulesToHere);
            }
        }
    }

    recursiveScannedModules.insert(module);
}

ConnectionGraph::ProccesingType ConnectionGraph::getProcessingType(int module) {
    if(forceSampleWise) return SampleWise;
    else if (recursiveModules.count(module)) return SampleWise;
    return BlockWise;
}

void ConnectionGraph::compileAllEntryPoints(std::vector<Instruction>& protoProgram, int module, std::set<int> &processedModules, std::set<int>& visitedModules)
{
    if (visitedModules.count(module)) return;
    visitedModules.insert(module);

    if (processedModules.count(module)) return;

    Module *m = getModule(module);
    
    std::set<int> connectedPads;

    for (int pad = 0; pad < m->getNumInputPads(); pad++) {
        for (int i = 0; i < cables.size(); ++i) {
            const auto* c = cables.at(i);
            if (c->isConnected(module, pad)) {
                auto fromModule = c->getFromModule();
                auto type = getProcessingType(fromModule);
                if (type == SampleWise) {
                    compileAllEntryPoints(protoProgram, fromModule, processedModules, visitedModules);
                }
                else if (type == BlockWise) {
                    compileModule(protoProgram, fromModule, processedModules, visitedModules);
                    if (!connectedPads.count(pad))
                        protoProgram.push_back(Instruction(OP_SET_OUTPUT_TO_INPUT, fromModule, c->getFromPad(), module, pad));
                    else
                        protoProgram.push_back(Instruction(OP_ADD_OUTPUT_TO_INPUT, fromModule, c->getFromPad(), module, pad));
                    connectedPads.insert(pad);
                }
            }
        }
    }
}

void ConnectionGraph::compileModule(std::vector<Instruction>& protoProgram, int module, std::set<int> &processedModules, std::set<int>& visitedModules)
{
    if (getProcessingType(module) == SampleWise) {
        compileAllEntryPoints(protoProgram, module, processedModules, visitedModules);
    }

    // Check if this module is already processed by the compiler
    if (processedModules.count(module)) return;
    processedModules.insert(module);

    Module *m = getModule(module);

    m->setFs(fsCompiled);

    // Iterate over all input pads

    std::set<int> connectedPads;
    for (int i = 0; i < cables.size(); ++i) {
        const auto* c = cables.at(i);

        // Check if other modules are connected to this pad
        for (int pad = 0; pad < m->getNumInputPads(); pad++) {
            if (c->isConnected(module, pad)) {
                auto fromModule = c->getFromModule();

                if (getProcessingType(module) == SampleWise && getProcessingType(fromModule) == BlockWise) {
                    assert(visitedModules.count(module)); // should be no need to pass visitedModules around
                    continue;
                }

                compileModule(protoProgram, fromModule, processedModules, visitedModules);

                if (!connectedPads.count(pad)) 
                    protoProgram.push_back(Instruction(OP_SET_OUTPUT_TO_INPUT, fromModule, c->getFromPad(), module, pad));
                else
                    protoProgram.push_back(Instruction(OP_ADD_OUTPUT_TO_INPUT, fromModule, c->getFromPad(), module, pad));

                connectedPads.insert(pad);
            }
        }
    }

    protoProgram.push_back(Instruction(OP_PROCESS, module));

}

void ConnectionGraph::processSample(int module, float fs)
{
    if (fs != fsCompiled) compilationStatus = NOT_COMPILED;
    if (module != compilationStatus) compileProgram(module, fs);

    float out = 0.0f;

    for(const Instruction &i : program) {
        switch(i.opcode) {
        case OP_PROCESS:
            modules[i.param0]->process();
            break;
        case OP_SET_OUTPUT_TO_INPUT:
            modules[i.param2]->sample_resetInput(i.param3);
        case OP_ADD_OUTPUT_TO_INPUT:
            out = modules[i.param0]->sample_getOutput(i.param1);
            modules[i.param2]->sample_addToInput(i.param3, out);
            break;
        default:
            assert(0); 
            break;
        }
    }
}

void ConnectionGraph::processBlock(int module, float fs) {
    if (fs != fsCompiled) compilationStatus = NOT_COMPILED;
    if (module != compilationStatus) compileProgram(module, fs);

    if (forceSampleWise) {
        assert(0); // won't work, as we've probably used setInputBlock ...
    }
    else {
        // process per block
        float buf[ConnectionGraph::k_blockSize] = { 0.f };
        float out = 0.0f;

        for (size_t j = 0; j < program.size(); ++j) {

            const auto& i = program.at(j);

            switch (i.opcode) {

            case OP_PROCESS:
                modules[i.param0]->process();
                break;
            case OP_CLEAR_INPUT:
                modules[i.param0]->sample_resetInput(i.param1);
                break;
            case OP_SET_OUTPUT_TO_INPUT:
                modules[i.param2]->sample_resetInput(i.param3);
            case OP_ADD_OUTPUT_TO_INPUT:
                out = modules[i.param0]->sample_getOutput(i.param1);
                modules[i.param2]->sample_addToInput(i.param3, out);
                break;

            case OP_X_UNBUFFER_CLEAR_INPUT:
                modules[i.param0]->unbuffer_clear(i.param1, i.param2);
                break;
            case OP_X_UNBUFFER_SET_INPUT:
                modules[i.param0]->unbuffer_set_input(i.param1, i.param2);
                break;
            case OP_X_UNBUFFER_ADD_INPUT:
                modules[i.param0]->unbuffer_add_input(i.param1, i.param2);
                break;
            case OP_X_BUFFER_CLEAR_OUTPUT:
                modules[i.param0]->buffer_clear(i.param1, i.param2);
                break;
            case OP_X_BUFFER_SET_OUTPUT:
                modules[i.param0]->buffer_set_output(i.param1, i.param2);
                break;
            case OP_X_BUFFER_ADD_OUTPUT:
                modules[i.param0]->buffer_add_output(i.param1, i.param2);
                break;

            case OP_B_PROCESS:
                modules[i.param0]->block_process();
                break;
            case OP_B_SET_OUTPUT_TO_INPUT:
                modules[i.param2]->block_resetInput(i.param3);
            case OP_B_ADD_OUTPUT_TO_INPUT:
                modules[i.param0]->block_getOutput(i.param1, buf);
                modules[i.param2]->block_addToInput(i.param3, buf);
                break;

            default:
                assert(0);
                break;
            }
        }
    }
}

void ConnectionGraph::makeModuleDocs(std::vector<PhasePhckr::ModuleDoc> &docList) {
    for (const auto & p : moduleRegister) {
        auto m = p.second();
        m->setName(p.first);
        docList.emplace_back(m->makeDoc());
        delete m;
    }
}
