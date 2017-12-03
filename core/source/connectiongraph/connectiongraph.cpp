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
};

ConnectionGraph::ConnectionGraph(bool forceSampleWise) 
    : compilationStatus(NOT_COMPILED)
    , forceSampleWise(forceSampleWise)
{

}

ConnectionGraph::ConnectionGraph(const ConnectionGraph& other)
    : compilationStatus(NOT_COMPILED)
    , forceSampleWise(other.forceSampleWise)
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
    // parse the graph once to find all modules involved in recursion loops
    moduleRecursionGroups.clear();

    if (!forceSampleWise) {
        findRecursionGroups(module, std::vector<int>());

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
    compileModule(protoProgram, module, processedModules);
    compilationStatus = module;

    if (!forceSampleWise) {
        std::cout << "protoProgram" << std::endl;
        printProgram(protoProgram);
        finalizeProgram(protoProgram);
    }
    else {
        program = protoProgram;
    }

    std::cout << "program" << std::endl;
    printProgram(program);
}

void ConnectionGraph::finalizeProgram(std::vector<Instruction>& protoProgram) {
    assert(!forceSampleWise);
    assert(program.size() == 0);

    std::map<int, std::set<int>> sampleWiseEntrypoints;

    for (int i = 0; i < protoProgram.size(); ++i) {
        auto instr = protoProgram.at(i);
        auto type = getProcessingType(instr.param0);

        if (type == SampleWise) 
        {
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
                                expandedSegment.push_back(Instruction(OP_X_UNBUFFER_INPUT, instr_.param0, port, n));
                            }
                        }
                        expandedSegment.push_back(instr_);
                        break;
                    case OP_SET_OUTPUT_TO_INPUT:
                        if (getProcessingType(instr_.param2) == BlockWise) {
                            expandedSegment.push_back(Instruction(OP_X_BUFFER_CLEAR, instr_.param0, instr_.param1, n));
                        }
                    case OP_ADD_OUTPUT_TO_INPUT:
                        if (getProcessingType(instr_.param2) == BlockWise) {
                            if (!postLoopInstructions.count(j)) {
                                auto instr__ = instr_;
                                instr__.opcode = instr__.opcode == OP_SET_OUTPUT_TO_INPUT ? OP_B_SET_OUTPUT_TO_INPUT : OP_B_ADD_OUTPUT_TO_INPUT;
                                postLoopInstructions[j] = instr__;
                            }
                            expandedSegment.push_back(Instruction(OP_X_BUFFER_OUTPUT, instr_.param0, instr_.param1, n));
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
            if (getProcessingType(instr.param2) == SampleWise && instr.opcode == OP_B_SET_OUTPUT_TO_INPUT || instr.opcode == OP_B_ADD_OUTPUT_TO_INPUT) {
                if (!sampleWiseEntrypoints.count(instr.param2)) sampleWiseEntrypoints[instr.param2] = std::set<int>();
                sampleWiseEntrypoints[instr.param2].insert(instr.param3);
            }
            program.emplace_back(instr);
        }
        else {
            assert(0);
        }
    }
}

void ConnectionGraph::printProgram(const vector<Instruction>& p) {
    for (int i = 0; i < p.size(); ++i) {
        const auto& instr = p[i];
        std::cout << i << ": ";
        switch (instr.opcode) {

        case OP_PROCESS:
            std::cout << "OP_PROCESS " << instr.param0; 
            break;
        case OP_SET_OUTPUT_TO_INPUT:
            std::cout << "OP_SET_OUTPUT_TO_INPUT " << instr.param0 << "," << instr.param1 << " -> " << instr.param2 << "," << instr.param3;
            break;
        case OP_ADD_OUTPUT_TO_INPUT:
            std::cout << "OP_ADD_OUTPUT_TO_INPUT " << instr.param0 << "," << instr.param1 << " -> " << instr.param2 << "," << instr.param3; 
            break;

        case OP_X_UNBUFFER_CLEAR:
            std::cout << "OP_X_UNBUFFER_CLEAR " << instr.param2 << " " << instr.param0 << "," << instr.param1;
            break;
        case OP_X_BUFFER_CLEAR:
            std::cout << "OP_X_BUFFER_CLEAR " << instr.param2 << " " << instr.param0 << "," << instr.param1;
            break;

        case OP_X_UNBUFFER_INPUT:
            std::cout << "OP_X_UNBUFFER_INPUT " << instr.param2 << " " << instr.param0 << "," << instr.param1;
            break;
        case OP_X_BUFFER_OUTPUT:
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

void ConnectionGraph::findRecursionGroups(int module, std::vector<int> processedModulesToHere) {
    bool foundSelf = false;
    for (auto otherModule : processedModulesToHere) {
        if (otherModule == module) foundSelf = true;
        if (foundSelf) {
            if (!moduleRecursionGroups.count(otherModule)) moduleRecursionGroups[otherModule] = std::set<int>();
            moduleRecursionGroups[otherModule].insert(module);
        }
    }
    if (foundSelf) return;
    processedModulesToHere.push_back(module);

    Module *m = getModule(module);
    for (int pad = 0; pad < m->getNumInputPads(); pad++) {
        for (const Cable *c : cables) {
            if (c->isConnected(module, pad)) {
                findRecursionGroups(c->getFromModule(), processedModulesToHere);
            }
        }
    }
}

ConnectionGraph::ProccesingType ConnectionGraph::getProcessingType(int module) {
    if(forceSampleWise) return SampleWise;
    else if (moduleRecursionGroups.count(module)) return SampleWise;
    return BlockWise;
}

void ConnectionGraph::compileModule(std::vector<Instruction>& protoProgram, int module, std::set<int> &processedModules)
{
    Module *m = getModule(module);

    // Check if this module is already processed by the compiler
    if (processedModules.count(module)) return;
    processedModules.insert(module);

    // Iterate over all input pads

    std::set<int> connectedPads;
    for (int i = 0; i < cables.size(); ++i) {
        const auto* c = cables.at(i);

        // Check if other modules are connected to this pad
        for (int pad = 0; pad < m->getNumInputPads(); pad++) {
            if (c->isConnected(module, pad)) {
                auto fromModule = c->getFromModule();

                compileModule(protoProgram, fromModule, processedModules);

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
    if (module != compilationStatus) compileProgram(module);

    float out = 0.0f;

    for(const Instruction &i : program) {
        switch(i.opcode) {
        case OP_PROCESS:
            modules[i.param0]->process((uint32_t)fs);
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

void ConnectionGraph::processBlock(int module, float fs, const vector<SampleBuffer>& inBuffers, vector<SampleBuffer>& outBuffers) {
    if (module != compilationStatus) compileProgram(module);

    const size_t inBufferSize = inBuffers.size();
    const size_t outBufferSize = outBuffers.size();

    // write input buffers
    for (size_t n = 0; n < inBufferSize; ++n) {
        auto* m = modules[inBuffers[n].module];
        auto p = inBuffers[n].pad;
        const auto* src = inBuffers[n].buf;
        m->block_setInput(p, src);
    }

    if (forceSampleWise) {
        // process per sample
        for (uint32_t i = 0; i < k_blockSize; ++i) {
            // unbuffer input sample
            for (size_t n = 0; n < inBufferSize; ++n) {
                auto* m = modules[inBuffers[n].module];
                auto p = inBuffers[n].pad;
                m->unbuffer_input(p, i);
            }

            // actual process
            processSample(module, fs);

            // buffer output sample
            for (size_t n = 0; n < outBufferSize; ++n) {
                auto* m = modules[outBuffers[n].module];
                auto p = outBuffers[n].pad;
                m->buffer_output(p, i);
            }
        }
    }
    else {
        // process per block
        float buf[ConnectionGraph::k_blockSize] = { 0.f };
        float out = 0.0f;

        for (const Instruction &i : program) {
            switch (i.opcode) {

            case OP_PROCESS:
                modules[i.param0]->process((uint32_t)fs);
                break;
            case OP_SET_OUTPUT_TO_INPUT:
                modules[i.param2]->sample_resetInput(i.param3);
            case OP_ADD_OUTPUT_TO_INPUT:
                out = modules[i.param0]->sample_getOutput(i.param1);
                modules[i.param2]->sample_addToInput(i.param3, out);
                break;

            case OP_X_UNBUFFER_CLEAR:
                modules[i.param0]->unbuffer_clear(i.param1, i.param2);
                break;
            case OP_X_UNBUFFER_INPUT:
                modules[i.param0]->unbuffer_input(i.param1, i.param2);
                break;
            case OP_X_BUFFER_CLEAR:
                modules[i.param0]->buffer_clear(i.param1, i.param2);
                break;
            case OP_X_BUFFER_OUTPUT:
                modules[i.param0]->buffer_output(i.param1, i.param2);
                break;

            case OP_B_PROCESS:
                modules[i.param0]->block_process((uint32_t)fs);
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

    // collect output buffers
    for (size_t n = 0; n < outBufferSize; ++n) {
        auto* m = modules[outBuffers[n].module];
        auto p = outBuffers[n].pad;
        auto* dst = outBuffers[n].buf;
        m->block_getOutput(p, dst);
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
