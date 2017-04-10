#ifndef CONNECTIONGRAPH_HPP
#define CONNECTIONGRAPH_HPP

#include <vector>
#include <string>

class Module;
class Cable;

class ConnectionGraph
{
protected:
    std::vector<Module*> modules;
    std::vector<Cable*> cables;
    uint32_t fs;
public:
    Module* getModule(int id);
    ConnectionGraph(uint32_t fs);
    virtual ~ConnectionGraph();
    int addModule(Module * module);
    int addModule(const char *type);
    void connect(int fromModule, std::string fromPad, int toModule, std::string toPad);
    void connect(int fromModule, int fromPad, int toModule, int toPad);
    void connect(int fromModule, int toModule, int toPad) { connect(fromModule, 0, toModule, toPad); }
    void connect(int fromModule, int toModule) { connect(fromModule, 0, toModule, 0); }
    void setInput(int module, int pad, float value);
    void process(int module, uint32_t time);
    float getOutput(int module, int pad);
};

#endif
