#include "connectiongraph.hpp"
#include <cmath>
#include <cstring>
#include <iostream>
#include <vector>
#include "moduleaccessor.hpp"

#include <assert.h>

class Cable {
   protected:
    int fromModule;
    int fromPad;
    int toModule;
    int toPad;

   public:
    Cable(int _fromModule, int _fromPad, int _toModule, int _toPad)
        : fromModule(_fromModule), fromPad(_fromPad), toModule(_toModule), toPad(_toPad) {}
    int getFromModule() const { return fromModule; }
    int getFromPad() const { return fromPad; }
    int getToModule() const { return toModule; }
    int getToPad() const { return toPad; }
    bool isConnected(int _toModule) const { return toModule == _toModule; }
    bool isConnected(int _toModule, int _toPad) const { return toModule == _toModule && toPad == _toPad; }
    bool isConnectedFrom(int _fromModule, int _fromPad) const {
        return fromModule == _fromModule && fromPad == _fromPad;
    }
};

ConnectionGraph::ConnectionGraph() {}

ConnectionGraph::ConnectionGraph(const ConnectionGraph& other)
    : compilationStatus(other.compilationStatus), program(other.program) {
    for (int i = 0; i < (int)other.modules.size(); ++i) {
        const auto& m = other.modules[i];
        Module* mCopy = m->clone();
        modules.push_back(mCopy);
    }
    for (int i = 0; i < (int)other.cables.size(); ++i) {
        const auto& c = other.cables[i];
        auto cCopy = new Cable(*c);
        cables.push_back(cCopy);
    }
}

ConnectionGraph::~ConnectionGraph() {
    for (Cable* c : cables) {
        delete c;
    }

    for (Module* m : modules) {
        delete m;
    }
}

Module* ConnectionGraph::getModule(int id) {
    if (id >= 0 && id < (int)modules.size()) {
        return modules[id];
    } else {
        std::cerr << "Error: No module with id '" << id << "'" << std::endl;
    }
    return nullptr;
}

int ConnectionGraph::addModule(std::string type) {
    compilationStatus.reset();
    int id = -1;
    Module* m = nullptr;

    for (auto mod : moduleRegister) {
        if (mod.first.compare(type) == 0) {
            m = mod.second();
            break;
        }
    }

    if (m) {
        id = (int)modules.size();
        modules.push_back(m);
        ModuleAccessor::setName(*m, type);
    } else {
        std::cerr << "Error: Module '" << type << "' not found" << std::endl;
    }

    return id;
}

int ConnectionGraph::addCustomModule(Module* module) {
    int id = (int)modules.size();
    modules.push_back(module);
    return id;
}

void ConnectionGraph::registerModule(std::string name, Module* (*moduleFactory)()) {
    moduleRegister.push_back(std::pair<std::string, Module* (*)()>(name, moduleFactory));
}

void ConnectionGraph::connect(int fromModule, std::string fromPad, int toModule, std::string toPad) {
    compilationStatus.reset();
    Module* mFrom = getModule(fromModule);
    Module* mTo = getModule(toModule);

    if (mFrom && mTo) {
        int fromPadNo = ModuleAccessor::getOutputPadFromName(*mFrom, fromPad);
        int toPadNo = ModuleAccessor::getInputPadFromName(*mTo, toPad);

        cables.push_back(new Cable(fromModule, fromPadNo, toModule, toPadNo));
    }
}

void ConnectionGraph::connect(int fromModule, int fromPad, int toModule, int toPad) {
    compilationStatus.reset();
    cables.push_back(new Cable(fromModule, fromPad, toModule, toPad));
}

void ConnectionGraph::setInputBlock(int module, int pad, const float* value) {
    ModuleAccessor::setInput(*modules[module], pad, value);
}

void ConnectionGraph::setInput(int module, int pad, float value) {
    ModuleAccessor::setInputFromSample(*modules[module], pad, value);
}

void ConnectionGraph::setInput(int module, std::string pad, float value) {
    auto* m = getModule(module);
    if (m) {
        auto p = ModuleAccessor::getInputPadFromName(*m, pad);
        if (p != -1) {
            ModuleAccessor::setInputFromSample(*m, p, value);
        }
    }
}

void ConnectionGraph::getOutputBlock(int module, int pad, float* buffer) {
    ModuleAccessor::getOutput(*modules[module], pad, buffer);
}

void ConnectionGraph::compileProgram(int module) {
    compilationStatus = module;
    program.clear();

    std::set<int> recursive_modules = findRecursiveModules(module);
    std::set<int> visited_modules;
    std::set<int> processed_modules;
    std::optional<int> loop_start;

    compileModule(module, recursive_modules, visited_modules, processed_modules, loop_start);

    if (loop_start) {
        program.push_back(Instruction(OP_LOOP, *loop_start));
    }
}

std::set<int> ConnectionGraph::findRecursiveModules(int module) {
    std::set<int> downstream_modules;
    std::set<int> recursive_modules;
    findRecursiveModulesInternal(module, recursive_modules, downstream_modules);
    return recursive_modules;
}

std::set<int> ConnectionGraph::findRecursiveModulesInternal(int module,
                                                            std::set<int>& recursive_modules,
                                                            std::set<int>& downstream_modules) {
    std::set<int> upstream_modules;
    if (!downstream_modules.count(module)) {
        downstream_modules.insert(module);

        // Find downstream modules.
        for (const Cable* c : cables) {
            if (c->isConnected(module)) {
                int fromModule = c->getFromModule();
                upstream_modules.merge(findRecursiveModulesInternal(fromModule, recursive_modules, downstream_modules));
            }
        }

        // Recursion found if there is an intersection between upstream and downstream modules.
        for (int upstream_module : upstream_modules) {
            if (downstream_modules.count(upstream_module)) {
                recursive_modules.insert(module);
                break;
            }
        }

        downstream_modules.erase(module);
    }
    upstream_modules.insert(module);
    return upstream_modules;
}

void ConnectionGraph::compileModule(int module,
                                    const std::set<int>& recursive_modules,
                                    std::set<int>& visited_modules,
                                    std::set<int>& processed_modules,
                                    std::optional<int>& loop_start) {
    if (visited_modules.count(module))
        return;
    visited_modules.insert(module);

    bool is_recursive = recursive_modules.count(module);
    std::set<int> initialized_pad;

    // Process input modules and copy/add input. First non-recursive and then recursive.
    for (bool recursive_dependency : {false, true}) {
        for (const Cable* c : cables) {
            if (!c->isConnected(module))
                continue;
            int from_module = c->getFromModule();
            int from_pad = c->getFromPad();
            int to_pad = c->getToPad();
            if ((recursive_modules.count(from_module) == 1) != recursive_dependency)
                continue;

            compileModule(from_module, recursive_modules, visited_modules, processed_modules, loop_start);

            if (recursive_dependency && is_recursive) {
                // Sample-wise processing.
                if (!loop_start)
                    loop_start = program.size();

                bool input_module_processed = processed_modules.count(from_module) == 1;

                // Copy or add input (sample-wise).
                if (!initialized_pad.count(to_pad)) {
                    initialized_pad.insert(to_pad);
                    program.push_back(Instruction(
                        input_module_processed ? OP_SAMPLE_SET_OUTPUT_TO_INPUT : OP_SAMPLE_SET_PREV_OUTPUT_TO_INPUT,
                        from_module, from_pad, module, to_pad));
                } else {
                    program.push_back(Instruction(
                        input_module_processed ? OP_SAMPLE_ADD_OUTPUT_TO_INPUT : OP_SAMPLE_ADD_PREV_OUTPUT_TO_INPUT,
                        from_module, from_pad, module, to_pad));
                }
            } else {
                if (recursive_dependency && !is_recursive) {
                    // Terminate loop if moving from sample to block processing.
                    program.push_back(Instruction(OP_LOOP, *loop_start));
                    loop_start.reset();
                }

                // Copy or add input (block-wise).
                if (!initialized_pad.count(to_pad)) {
                    initialized_pad.insert(to_pad);
                    program.push_back(Instruction(OP_BLOCK_SET_OUTPUT_TO_INPUT, from_module, from_pad, module, to_pad));
                } else {
                    program.push_back(Instruction(OP_BLOCK_ADD_OUTPUT_TO_INPUT, from_module, from_pad, module, to_pad));
                }
            }
        }
    }

    // Process the current module.
    processed_modules.insert(module);
    if (is_recursive) {
        // Sample-wise processing.
        if (!loop_start)
            loop_start = program.size();
        program.push_back(Instruction(OP_SAMPLE_PROCESS, module));
    } else {
        // Block-wise processing.
        program.push_back(Instruction(OP_BLOCK_PROCESS, module));
    }
}

void ConnectionGraph::processBlock(int module, float sampleRate) {
    if (module != *compilationStatus)
        compileProgram(module);
    if (sampleRate != fs)
        setSamplerate(sampleRate);

    int ic = 0;
    int program_size = program.size();
    int sample = 0;
    int prev_sample = k_blockSize - 1;

    while (ic < program_size) {
        const auto& i = program[ic];

        switch (i.opcode) {
            case OP_BLOCK_SET_OUTPUT_TO_INPUT:
                ModuleAccessor::setBlockOutputToInput(*modules[i.param0], i.param1, *modules[i.param2], i.param3);
                break;
            case OP_BLOCK_ADD_OUTPUT_TO_INPUT:
                ModuleAccessor::addBlockOutputToInput(*modules[i.param0], i.param1, *modules[i.param2], i.param3);
                break;
            case OP_BLOCK_PROCESS:
                ModuleAccessor::processBlock(*modules[i.param0]);
                break;
            case OP_SAMPLE_SET_OUTPUT_TO_INPUT:
                ModuleAccessor::setSampleOutputToInput(*modules[i.param0], i.param1, sample, *modules[i.param2],
                                                       i.param3, sample);
                break;
            case OP_SAMPLE_ADD_OUTPUT_TO_INPUT:
                ModuleAccessor::addSampleOutputToInput(*modules[i.param0], i.param1, sample, *modules[i.param2],
                                                       i.param3, sample);
                break;
            case OP_SAMPLE_SET_PREV_OUTPUT_TO_INPUT:
                ModuleAccessor::setSampleOutputToInput(*modules[i.param0], i.param1, prev_sample, *modules[i.param2],
                                                       i.param3, sample);
                break;
            case OP_SAMPLE_ADD_PREV_OUTPUT_TO_INPUT:
                ModuleAccessor::addSampleOutputToInput(*modules[i.param0], i.param1, prev_sample, *modules[i.param2],
                                                       i.param3, sample);
                break;
            case OP_SAMPLE_PROCESS:
                ModuleAccessor::processSample(*modules[i.param0], sample);
                break;
            case OP_LOOP:
                // Loop until a full block is processed.
                if (++sample < k_blockSize) {
                    prev_sample = prev_sample < k_blockSize - 1 ? prev_sample + 1 : 0;
                    ic = i.param0;
                    continue;
                } else {
                    sample = 0;
                    prev_sample = k_blockSize - 1;
                }
                break;

            default:
                assert(0);
                break;
        }

        ++ic;
    }
}

void ConnectionGraph::setSamplerate(float fs_) {
    fs = fs_;
    for (int i = 0; i < modules.size(); i++) {
        ModuleAccessor::setFs(*modules[i], fs);
    }
}

void ConnectionGraph::makeModuleDocs(std::vector<PhasePhdgr::ModuleDoc>& docList) {
    for (const auto& p : moduleRegister) {
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
    compilationStatus.reset();
}

void ConnectionGraph::troubleshoot() {
    // TODO, turn this hack into some problem report thing which a
    // gui can use to visually represent problematic pieces of graph

    auto check_value = [&](float value, const std::string& unit) {
        if (unit == "hz")
            return fabsf(value) < fs / 2;
        if (unit == "bpm")
            return value >= 0;
        if (unit == "ppq")
            return value >= 0;
        if (unit == "seconds")
            return value >= 0;
        return fabsf(value) < 5.0f;  // TODO, share with synth.cpp
    };

    auto check_pads = [&](const std::vector<Pad>& pads) {
        std::vector<std::pair<size_t, float>> problems;
        for (auto j = 0u; j < pads.size(); j++) {
            auto& pad = pads.at(j);
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
            std::cerr << "input @ Module " << mi << " (" << ModuleAccessor::getName(*modules.at(mi)) << ") " << name
                      << " (" << p.first << ") = " << p.second << " " << unit << std::endl;
        }
        for (const auto& p : oprobs) {
            auto unit = ModuleAccessor::getOutputs(*modules.at(mi))[p.first].unit;
            auto name = ModuleAccessor::getOutputs(*modules.at(mi))[p.first].name;
            std::cerr << "output @ Module " << mi << " (" << ModuleAccessor::getName(*modules.at(mi)) << ") " << name
                      << " (" << p.first << ") = " << p.second << " " << unit << std::endl;
        }
    }
}
