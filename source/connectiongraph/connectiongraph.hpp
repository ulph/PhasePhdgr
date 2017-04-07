#ifndef CONNECTIONGRAPH_HPP
#define CONNECTIONGRAPH_HPP

#include <vector>

class Module;
class Cable;

class ConnectionGraph
{
protected:
    std::vector<Module*> modules;
    std::vector<Cable*> cables;
    Module* getModule(int id);
    uint32_t fs;
public:
    ConnectionGraph(uint32_t fs);
    int addModule(const char *type);
    void connect(int fromModule, int fromPad, int toModule, int toPad);
    void connect(int fromModule, int toModule, int toPad) { connect(fromModule, 0, toModule, toPad); }
    void connect(int fromModule, int toModule) { connect(fromModule, 0, toModule, 0); }
    void setInput(int module, int pad, float value);
    void process(int module, float time);
    float getOutput(int module, int pad);
};

#endif
