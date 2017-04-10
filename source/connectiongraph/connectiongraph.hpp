#ifndef CONNECTIONGRAPH_HPP
#define CONNECTIONGRAPH_HPP

#include <vector>
#include <string>

class Module;
class Cable;

enum Opcode { OP_PROCESS, OP_RESET_INPUT, OP_ADD_OUTPUT_TO_INPUT };

struct Instruction {
    int opcode;
    int param0;
    int param1;
    int param2;
    int param3;
    
    Instruction(int opcode, int param0, int param1, int param2, int param3) : opcode(opcode), param0(param0), param1(param1), param2(param2), param3(param3) {}
    Instruction(int opcode, int param0, int param1) : opcode(opcode), param0(param0), param1(param1), param2(0), param3(0) {}
    Instruction(int opcode, int param0) : opcode(opcode), param0(param0), param1(0), param2(0), param3(0) {}
};

class ConnectionGraph
{
protected:
    std::vector<Module*> modules;
    std::vector<Cable*> cables;
    std::vector<Instruction> program;
    uint32_t fs;
    int compiledForModule;
    void compileProgram(int module);
    void compileModule(int module);
public:
    Module* getModule(int id);
    ConnectionGraph(uint32_t fs);
    virtual ~ConnectionGraph();
    int addModule(const char *type);
    void connect(int fromModule, std::string fromPad, int toModule, std::string toPad);
    void connect(int fromModule, int fromPad, int toModule, int toPad);
    void connect(int fromModule, int toModule, int toPad) { connect(fromModule, 0, toModule, toPad); }
    void connect(int fromModule, int toModule) { connect(fromModule, 0, toModule, 0); }
    void setInput(int module, int pad, float value);
    float getOutput(int module, int pad);
    void process(int module);
};

#endif
