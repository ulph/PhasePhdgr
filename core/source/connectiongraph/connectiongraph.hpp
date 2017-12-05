#ifndef CONNECTIONGRAPH_HPP
#define CONNECTIONGRAPH_HPP

#include <vector>
#include <string>
#include <set>
#include <functional>

#include "phasephckr/docs.hpp"

#include "instruction.hpp"

class Module;
class Cable;

class ConnectionGraph
{
protected:
    std::vector<std::pair<std::string, Module* (*)()>> moduleRegister;
    std::vector<Module*> modules;
    std::vector<Cable*> cables;
    std::vector<Instruction> program;
    int compilationStatus;
    void findRecursionGroups(int module, std::vector<int> processedModulesToHere);
    void compileAllEntryPoints(std::vector<Instruction>& protoProgram, int module, std::set<int> &processedModules, std::set<int>& visitedModules);
    void compileModule(std::vector<Instruction>& protoProgram, int module, std::set<int> &processedModules, std::set<int>& visitedModules);
    void finalizeProgram(std::vector<Instruction>& protoProgram);

    Module* getModule(int id);

    void printProgram(const vector<Instruction>& p);

    enum ProccesingType{
        BlockWise,
        SampleWise,
    };

    std::map<int, set<int>> moduleRecursionGroups;
    ProccesingType getProcessingType(int module);

    const bool forceSampleWise;

    float fsCompiled = 48000.f;

public:
    static const int k_blockSize = 64;
    struct SampleBuffer {
        int module;
        int pad;
        float buf[k_blockSize];
    };

    ConnectionGraph(bool forceSampleWise=false);
    ConnectionGraph(const ConnectionGraph& other);
    virtual ~ConnectionGraph();
    int addModule(std::string type);
    int addCustomModule(Module* module);
    void registerModule(std::string name, Module* (*moduleFactory)());
    void connect(int fromModule, std::string fromPad, int toModule, std::string toPad);
    void connect(int fromModule, int fromPad, int toModule, int toPad);
    void connect(int fromModule, int toModule, int toPad) { connect(fromModule, 0, toModule, toPad); }
    void connect(int fromModule, int toModule) { connect(fromModule, 0, toModule, 0); }
    void setInput(int module, int pad, float value);
    void setInput(int module, std::string pad, float value);
    float getOutput(int module, int pad);
    void processSample(int module, float fs);
    void processBlock(int module, float fs, const vector<SampleBuffer>& inBuffers, vector<SampleBuffer>& outBuffers);
    void compileProgram(int module, float fs);
    void makeModuleDocs(std::vector<PhasePhckr::ModuleDoc> &docList);
};

#endif
