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
    std::set<int> recursiveModules;
    std::set<int> recursiveScannedModules;
    int recursivePathsSkipped = 0;
    const bool forceSampleWise;
    float fs = -1.0;
    void findRecursions(int module, std::vector<int> processedModulesToHere);
    void compileAllEntryPoints(std::vector<Instruction>& protoProgram, int module, std::set<int> &processedModules, std::set<int>& visitedModules);
    void compileModule(std::vector<Instruction>& protoProgram, int module, std::set<int> &processedModules, std::set<int>& visitedModules);
    void finalizeProgram(std::vector<Instruction>& protoProgram);
    Module* getModule(int id);
    enum ProccesingType {
        BlockWise,
        SampleWise,
    };
    ProccesingType getProcessingType(int module);
    bool validateProgram(const std::vector<Instruction>& program);

public:
    static const int k_blockSize = 64;
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
    void setInputBlock(int module, int pad, const float* buffer);
    void setInput(int module, int pad, float value);
    void setInput(int module, std::string pad, float value);
    float getOutput(int module, int pad);
    void getOutputBlock(int module, int pad, float* buffer);
    void processSample(int module, float sampleRate);
    void processBlock(int module, float sampleRate);
    void compileProgram(int module);
    void setSamplerate(float fs);
    void makeModuleDocs(std::vector<PhasePhckr::ModuleDoc> &docList);
    const std::vector<Instruction>& dumpProgragram();
    int recallProgram(const std::vector<Instruction>& program);
};

#endif
