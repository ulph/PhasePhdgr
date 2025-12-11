#ifndef CONNECTIONGRAPH_HPP
#define CONNECTIONGRAPH_HPP

#include <optional>
#include <string>
#include <vector>
#include "instruction.hpp"
#include "module.hpp"
#include "phasephdgr/docs.hpp"

class Cable;

class ConnectionGraph {
   protected:
    std::vector<std::pair<std::string, Module* (*)()>> moduleRegister;
    std::vector<Module*> modules;
    std::vector<Cable*> cables;
    std::vector<Instruction> program;
    std::optional<int> compilationStatus;

    float fs = -1.0;
    std::set<int> findRecursiveModules(int module);
    std::set<int> findRecursiveModulesInternal(int module,
                                               std::set<int>& recursive_modules,
                                               std::set<int>& upstream_modules);
    void compileModule(int module,
                       const std::set<int>& recursive_modules,
                       std::set<int>& visited_modules,
                       std::set<int>& processed_modules,
                       std::optional<int>& loop_start);
    Module* getModule(int id);

   public:
    static const int k_blockSize = Pad::k_blockSize;
    ConnectionGraph();
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
    void getOutputBlock(int module, int pad, float* buffer);
    void processBlock(int module, float sampleRate);
    void compileProgram(int module);
    void setSamplerate(float fs);
    void makeModuleDocs(std::vector<PhasePhdgr::ModuleDoc>& docList);
    void reset();
    void troubleshoot();
};

#endif
