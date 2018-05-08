#ifndef CONNECTIONGRAPH_HPP
#define CONNECTIONGRAPH_HPP

#include <vector>
#include <string>
#include <set>
#include <functional>

#include "phasephckr/docs.hpp"

#include "module.hpp"

#include "instruction.hpp"

class Cable;

class ConnectionGraph
{
protected:
    std::vector<std::pair<std::string, Module* (*)()>> moduleRegister;
    std::vector<Module*> modules;
    std::vector<Cable*> cables;
    std::vector<Instruction> protoProgram;
    std::vector<Instruction> program;
    int compilationStatus;
    std::map<int, int> recursiveModuleGroups; // aka, colors
    std::set<int> recursiveScannedModules;
    std::set<std::pair<int,int>> recursiveTraversedConnections;
    int modulesVisitedInFindRecursions = 0;
    std::set<int> visitedModules;
    std::set<int> processedModules;
    std::map<int, std::set<int>> sampleWiseEntrypoints;
    std::map<int, std::set<int>> sampleWiseExitpoints;

    const bool forceSampleWise;
    float fs = -1.0;
    void findRecursions(int module, std::vector<int> processedModulesToHere);
    void compileAllEntryPoints(int module);
    void compileModule(int module);
    void finalizeProgram();
    void buildRepeatedSampleWiseSegment(int& i);
    void addSampleWiseToBlockWise(const Instruction& instr);
    void addBlockWiseToSampleWise(const Instruction& instr);
    Module* getModule(int id);
    enum ProccesingType {
        BlockWise,
        SampleWise,
    };
    ProccesingType getProcessingType(int module);

public:
    static const int k_blockSize = Pad::k_blockSize;
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
    void reset();
};

#endif
