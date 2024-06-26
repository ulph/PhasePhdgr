#include <iostream>
#include <vector>
#include <algorithm> 
#include <cmath>
#include <cstring>
#include <sstream>
#include "connectiongraph.hpp"
#include "moduleaccessor.hpp"

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
        ModuleAccessor::setName(*m, type);
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
        int fromPadNo = ModuleAccessor::getOutputPadFromName(*mFrom, fromPad);
        int toPadNo = ModuleAccessor::getInputPadFromName(*mTo, toPad);
        
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
    ModuleAccessor::block_setInput(*modules[module], pad, value);
}

void ConnectionGraph::setInput(int module, int pad, float value)
{
    ModuleAccessor::setInput(*modules[module], pad, value);
}

void ConnectionGraph::setInput(int module, std::string pad, float value){
    auto *m = getModule(module);
    if (m) {
        auto p = ModuleAccessor::getInputPadFromName(*m, pad);
        if (p != -1) {
            ModuleAccessor::setInput(*m, p, value);
        }
    }
}

float ConnectionGraph::getOutput(int module, int pad)
{
    return ModuleAccessor::sample_getOutput(*modules[module], pad);
}

void ConnectionGraph::getOutputBlock(int module, int pad, float* buffer)
{
    ModuleAccessor::block_getOutput(*modules[module], pad, buffer);
}

void ConnectionGraph::compileProgram(int module)
{
    // parse the graph once to find all modules involved in recursion loops
    recursiveModuleGroups.clear();
    recursiveScannedModules.clear();
    recursiveTraversedConnections.clear();
    modulesVisitedInFindRecursions = 0;

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

        assert(recursiveModuleGroups.size() <= modules.size());
        assert(recursiveScannedModules.size() <= modules.size());
        assert(recursiveTraversedConnections.size() <= cables.size());

    }

    program.clear();
    protoProgram.clear();

    visitedModules.clear();
    processedModules.clear();

    compileModule( module);
    compilationStatus = module;

    if (!forceSampleWise) {
        sampleWiseEntrypoints.clear();
        sampleWiseExitpoints.clear();
        finalizeProgram();
    }
}

void ConnectionGraph::buildRepeatedSampleWiseSegment(int& i) {
    auto instr = protoProgram.at(i);
    auto fromType = getProcessingType(instr.param0);

    std::vector<Instruction> segment;
    while (i < protoProgram.size()) {
        instr = protoProgram.at(i);
        fromType = getProcessingType(instr.param0);
        if (fromType == BlockWise) break;
        segment.push_back(instr);
        i++;
    }
    if (segment.size() == 0) return;

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
                            expandedSegment.push_back(Instruction(OP_S_CLEAR_INPUT, instr_.param0, port));
                        }
                    }
                    if (sampleWiseExitpoints.count(instr_.param0)) {
                        for (int port : sampleWiseExitpoints.at(instr_.param0)) {
                            expandedSegment.push_back(Instruction(OP_X_BUFFER_SET_OUTPUT, instr_.param0, port, n));
                        }
                    }
                    break;

                case OP_SET_OUTPUT_TO_INPUT:
                case OP_ADD_OUTPUT_TO_INPUT:
                    if (getProcessingType(instr_.param2) == BlockWise) {
                        if (!postLoopInstructions.count(j)) {
                            auto instr__ = instr_;
                            instr__.opcode = instr__.opcode == OP_SET_OUTPUT_TO_INPUT ? OP_B_SET_OUTPUT_TO_INPUT : OP_B_ADD_OUTPUT_TO_INPUT;
                            postLoopInstructions[j] = instr__;
                        }
                        if (!sampleWiseExitpoints.count(instr_.param0)) sampleWiseExitpoints[instr_.param0] = std::set<int>();
                        if (!sampleWiseExitpoints.at(instr_.param0).count(instr_.param1)) {
                            sampleWiseExitpoints[instr_.param0].insert(instr_.param1);
                            expandedSegment.push_back(Instruction(OP_X_BUFFER_SET_OUTPUT, instr_.param0, instr_.param1, n));
                        }
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
    for (const auto& kv : postLoopInstructions){
        program.push_back(kv.second);
    }
}

void ConnectionGraph::addSampleWiseToBlockWise(const Instruction& instr) {
    auto instr__ = instr;
    instr__.opcode = instr.opcode == OP_ADD_OUTPUT_TO_INPUT ? OP_B_ADD_OUTPUT_TO_INPUT : OP_B_SET_OUTPUT_TO_INPUT;
    program.push_back(instr__);
}

void ConnectionGraph::addBlockWiseToSampleWise(const Instruction& instr) {
    if (!sampleWiseEntrypoints.count(instr.param2)) sampleWiseEntrypoints[instr.param2] = std::set<int>();
    sampleWiseEntrypoints[instr.param2].insert(instr.param3);
    if (instr.opcode == OP_B_SET_OUTPUT_TO_INPUT) {
        program.emplace_back(Instruction(OP_S_CLEAR_INPUT, instr.param2, instr.param3));
    }
}

void ConnectionGraph::finalizeProgram() {
    assert(!forceSampleWise);
    assert(program.size() == 0);

    for (int i = 0; i < protoProgram.size(); ++i) {

        auto instr = protoProgram.at(i);
        auto fromType = getProcessingType(instr.param0);

        if (fromType == SampleWise){
            auto toType = getProcessingType(instr.param2);
            if (
                instr.opcode == OP_PROCESS
                ||
                (instr.opcode == OP_SET_OUTPUT_TO_INPUT && toType == SampleWise)
                ||
                (instr.opcode == OP_ADD_OUTPUT_TO_INPUT && toType == SampleWise)
                )
            {
                // enter segment
                buildRepeatedSampleWiseSegment(i);
            }
            else if (
                (instr.opcode == OP_ADD_OUTPUT_TO_INPUT && toType == BlockWise)
                || 
                (instr.opcode == OP_SET_OUTPUT_TO_INPUT && toType == BlockWise)
                )
            {
                // samplewise->blockwise add/set not in a segment
                addSampleWiseToBlockWise(instr);
                i++;
            }
            else {
                assert(0);
            }
        }

        if (i >= protoProgram.size()) break;

        instr = protoProgram.at(i);
        fromType = getProcessingType(instr.param0);

        if (fromType == BlockWise) 
        {
            auto toType = getProcessingType(instr.param2);
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

            if (
                (instr.opcode == OP_B_SET_OUTPUT_TO_INPUT && toType == SampleWise)
                || 
                (instr.opcode == OP_B_ADD_OUTPUT_TO_INPUT && toType == SampleWise)
            ){
                // blockwise->samplewise add/set -- transition bookkeeping / buffering
                addBlockWiseToSampleWise(instr);
            }
        }
        else {
            assert(0);
        }
    }
}

void ConnectionGraph::findRecursions(int module, std::vector<int> processedModulesToHere) {
    assert(processedModulesToHere.size() <= modules.size()); // module can be twice at max (and all else once)

    modulesVisitedInFindRecursions++;
    bool foundSelf = false;
    std::set<int> groupsToHere;

    bool doneDone = true;
    
    for (auto otherModule : processedModulesToHere) {
        if (otherModule == module) foundSelf = true;

        if (recursiveModuleGroups.count(otherModule)) groupsToHere.insert(recursiveModuleGroups.at(otherModule));

        if (groupsToHere.count(module)) foundSelf = true;

        if (recursiveModuleGroups.count(module)) {
            if (groupsToHere.count(recursiveModuleGroups.at(module))) foundSelf = true;
        }

        if (foundSelf) {
            if (recursiveModuleGroups.count(otherModule) && recursiveModuleGroups.at(otherModule) != module) {
                // repaint (all affected)
                auto toReplace = recursiveModuleGroups.at(otherModule);
                for (auto& kv : recursiveModuleGroups) {
                    if (kv.second == toReplace) kv.second = module;
                }
            }
            else {
                // paint
                recursiveModuleGroups[otherModule] = module;
            }
        }

        doneDone = doneDone && recursiveScannedModules.count(otherModule);
    }

    if (foundSelf) return;
    processedModulesToHere.push_back(module);

    Module *m = getModule(module);
    for (int pad = 0; pad < ModuleAccessor::getNumInputPads(*m); pad++) {
        for (const Cable *c : cables) {
            if (c->isConnected(module, pad)) {
                auto from = c->getFromModule();
                if (!(doneDone && recursiveScannedModules.count(from))) {
                    auto c = make_pair(from, module);
                    if (!recursiveTraversedConnections.count(c)) {
                        findRecursions(from, processedModulesToHere);
                        recursiveTraversedConnections.insert(c);
                    }
                }
            }
        }
    }

    recursiveScannedModules.insert(module);
}

ConnectionGraph::ProccesingType ConnectionGraph::getProcessingType(int module) {
    if(forceSampleWise) return SampleWise;
    else if (recursiveModuleGroups.count(module)) return SampleWise;
    return BlockWise;
}

void ConnectionGraph::compileAllEntryPoints(int module)
{
    // TODO, there must be a way to bake this into compileModule

    assert(getProcessingType(module) == SampleWise);

    // Allready visited this module 
    if (visitedModules.count(module)) return;
    visitedModules.insert(module);

    // Allready processed this module
    if (processedModules.count(module)) return;

    Module *m = getModule(module);
    
    std::set<int> connectedPads;

    // Iterate over all input pads (order doesn't matter as objective is to skip ahead to and processes all entry points)
    for (int pad = 0; pad < ModuleAccessor::getNumInputPads(*m); pad++) {
        for (int i = 0; i < cables.size(); ++i) {
            const auto* c = cables.at(i);
            if (c->isConnected(module, pad)) {
                auto fromModule = c->getFromModule();
                auto fromType = getProcessingType(fromModule);
                if (fromType == SampleWise) {
                    // Keep skipping ahead
                    compileAllEntryPoints(fromModule);
                }
                else if (fromType == BlockWise) {
                    // From here, compile as usual
                    compileModule(fromModule);
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

void ConnectionGraph::compileModule(int module)
{
    // Do special stuff when entering a SampleWise module - we need to skip ahead
    if (getProcessingType(module) == SampleWise) {
        compileAllEntryPoints(module);
    }

    // Allready processed this module
    if (processedModules.count(module)) return;
    processedModules.insert(module);

    Module *m = getModule(module);

    // Iterate over all input pads (order of cables matters, we need to always enter any SampleWise modules first)
    std::set<int> connectedPads;
    for (int i = 0; i < cables.size(); ++i) {
        const auto* c = cables.at(i);
        if (c->getToModule() == module) {
            // Check if other modules are connected to this pad
            for (int pad = 0; pad < ModuleAccessor::getNumInputPads(*m); pad++) {
                if (c->isConnected(module, pad)) {
                    auto fromModule = c->getFromModule();

                    if (getProcessingType(module) == SampleWise && getProcessingType(fromModule) == BlockWise) {
                        // allready processed via compileAllEntryPoints
                        continue;
                    }

                    compileModule(fromModule);

                    if (!connectedPads.count(pad))
                        protoProgram.push_back(Instruction(OP_SET_OUTPUT_TO_INPUT, fromModule, c->getFromPad(), module, pad));
                    else
                        protoProgram.push_back(Instruction(OP_ADD_OUTPUT_TO_INPUT, fromModule, c->getFromPad(), module, pad));

                    connectedPads.insert(pad);
                }
            }
        }
    }

    protoProgram.push_back(Instruction(OP_PROCESS, module));

}

void ConnectionGraph::processSample(int module, float sampleRate)
{
    if (module != compilationStatus) compileProgram(module);
    if (sampleRate != fs) setSamplerate(sampleRate);

    float out = 0.0f;

    for(const Instruction &i : protoProgram) {
        switch(i.opcode) {
        case OP_PROCESS:
            ModuleAccessor::process(*modules[i.param0]);
            break;
        case OP_SET_OUTPUT_TO_INPUT:
            out = ModuleAccessor::sample_getOutput(*modules[i.param0], i.param1);
            ModuleAccessor::sample_setInput(*modules[i.param2], i.param3, out);
            break;
        case OP_ADD_OUTPUT_TO_INPUT:
            out = ModuleAccessor::sample_getOutput(*modules[i.param0], i.param1);
            ModuleAccessor::sample_addToInput(*modules[i.param2], i.param3, out);
            break;
        default:
            assert(0); 
            break;
        }
    }
}

void ConnectionGraph::processBlock(int module, float sampleRate) {
    if (module != compilationStatus) compileProgram(module);
    if (sampleRate != fs) setSamplerate(sampleRate);

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
                ModuleAccessor::process(*modules[i.param0]);
                break;
            case OP_SET_OUTPUT_TO_INPUT:
                out = ModuleAccessor::sample_getOutput(*modules[i.param0], i.param1);
                ModuleAccessor::sample_setInput(*modules[i.param2], i.param3, out);
                break;
            case OP_ADD_OUTPUT_TO_INPUT:
                out = ModuleAccessor::sample_getOutput(*modules[i.param0], i.param1);
                ModuleAccessor::sample_addToInput(*modules[i.param2], i.param3, out);
                break;

            case OP_S_CLEAR_INPUT:
                ModuleAccessor::sample_resetInput(*modules[i.param0], i.param1);
                break;
            case OP_X_UNBUFFER_ADD_INPUT:
                ModuleAccessor::unbuffer_add_input(*modules[i.param0], i.param1, i.param2);
                break;
            case OP_X_BUFFER_SET_OUTPUT:
                ModuleAccessor::buffer_set_output(*modules[i.param0], i.param1, i.param2);
                break;

            case OP_B_PROCESS:
                ModuleAccessor::block_process(*modules[i.param0]);
                break;
            case OP_B_SET_OUTPUT_TO_INPUT:
                ModuleAccessor::block_getOutput(*modules[i.param0], i.param1, buf);
                ModuleAccessor::block_setInput(*modules[i.param2], i.param3, buf);
                break;
            case OP_B_ADD_OUTPUT_TO_INPUT:
                ModuleAccessor::block_getOutput(*modules[i.param0], i.param1, buf);
                ModuleAccessor::block_addToInput(*modules[i.param2], i.param3, buf);
                break;

            default:
                assert(0);
                break;
            }
        }
    }
}

void ConnectionGraph::setSamplerate(float fs_) {
    fs = fs_;
    for (int i = 0; i < modules.size(); i++) {
        ModuleAccessor::setFs(*modules[i], fs);
    }
}

void ConnectionGraph::makeModuleDocs(std::vector<PhasePhdgr::ModuleDoc> &docList) {
    for (const auto & p : moduleRegister) {
        auto m = p.second();
        auto d = PhasePhdgr::ModuleDoc();
        ModuleAccessor::setName(*m, p.first);
        d.fromModule(m);
        docList.emplace_back(d);
        delete m;
    }
}

void ConnectionGraph::reset() {
    for (auto i = 0u; i < modules.size(); i++) {
        auto m = modules[i]->clone();
        delete modules[i];
        modules[i] = m;
    }
    compilationStatus = NOT_COMPILED;
}

void ConnectionGraph::troubleshoot() {
    // TODO, turn this hack into some problem report thing which a 
    // gui can use to visually represent problematic pieces of graph

    auto check_value = [&](float value, const std::string& unit) {
        if (unit == "hz") return fabsf(value) < fs/2;
        if (unit == "bpm") return value >= 0;
        if (unit == "ppq") return value >= 0;
        if (unit == "seconds") return value >= 0;
        return fabsf(value) < 5.0f; // TODO, share with synth.cpp
    };

    auto check_pads = [&](const std::vector<Pad> & pads) {
        std::vector<std::pair<size_t, float>> problems;
        for (auto j = 0u; j < pads.size(); j++) {
            auto& pad = pads.at(j);
            if (!check_value(pad.value, pad.unit)) {
                problems.push_back(std::make_pair(j, pad.value)); 
                continue;
            }
            for (auto n = 0u; n < Pad::k_blockSize; n++) {
                if (!check_value(pad.values[n], pad.unit)) {
                    problems.push_back(std::make_pair(j, pad.values[n]));
                    break;
                }
            }
        }
        return problems;
    };

    for (auto mi = 0u; mi < modules.size(); mi++) {
        auto m = modules[mi];
        auto iprobs = check_pads(ModuleAccessor::getInputs(*m));
        auto oprobs = check_pads(ModuleAccessor::getOutputs(*m));
        for (const auto& p : iprobs) {
            auto unit = ModuleAccessor::getInputs(*modules.at(mi))[p.first].unit;
            auto name = ModuleAccessor::getInputs(*modules.at(mi))[p.first].name;
            std::cerr << "input @ Module "  << mi << " (" << ModuleAccessor::getName(*modules.at(mi)) << ") " << name << " (" << p.first << ") = " << p.second << " " << unit << std::endl;
        }
        for (const auto& p : oprobs) {
            auto unit = ModuleAccessor::getOutputs(*modules.at(mi))[p.first].unit;
            auto name = ModuleAccessor::getOutputs(*modules.at(mi))[p.first].name;
            std::cerr << "output @ Module " << mi << " (" << ModuleAccessor::getName(*modules.at(mi)) << ") " << name << " (" << p.first << ") = " << p.second << " " << unit << std::endl;
        }
    }

}