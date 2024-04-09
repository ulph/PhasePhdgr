#include <iostream>
#include <string.h> 

#include "pluginloader.hpp"
#include "pluginsregister.hpp"
#include "connectiongraph.hpp"

#define TEST(condition, msg_on_fail) \
if(!condition) { \
  std::cerr << "TEST failed: " <<__FILE__ << ":" << __LINE__ << " " << #condition " " << msg_on_fail << std::endl;\
  exit(-1);\
} \
else { \
  std::cout << "TEST passed: " << __FILE__ << ":" << __LINE__ << " " << #condition << std::endl;\
}

int main(int argc, const char* argv[])
{
    std::string p = BuildDylibName("plugin_example"); // see example.cpp

    {
        PhasePhdgr::PluginLoader ex(p.c_str());
        auto d = ex.getData();
        TEST(d, p << " failed to load");

        std::cout << "Loaded plug-in: " << d->getName() << " (filename: " << p << ")" << std::endl;

        TEST((strcmp("sdk_ex", d->getName()) == 0), "unexpected plugin name " << d->getName());

        ModuleFactoryMap m;

        d->enumerateFactories(m);

        TEST(m.size(), "No factories enumerated!");

        for (const auto& kv : m) {
            auto m = kv.second();
            std::cout << kv.first << " - " << m->docString() << std::endl;
        }

        TEST(m.count("ex_mod"), "Expected to find module in example plugin");

        ConnectionGraph cg;

        for (const auto& kv : m) {
            cg.registerModule(kv.first, kv.second);
            int handle = cg.addModule(kv.first);
            std::cout << "added " << kv.first << " -> " << handle << std::endl;

            auto out = cg.getOutput(handle, 0);
            std::cout << "output (pad 0): " << out << std::endl;
            cg.setInput(handle, 0, 42.0f);
            std::cout << "set 42 (pad 0)" << std::endl;
            cg.processSample(handle, 48000.0f);
            std::cout << "process" << std::endl;
            out = cg.getOutput(handle, 0);
            std::cout << "output (pad 0): " << out << std::endl;
        }
    }

    {
        PhasePhdgr::PluginsRegister pr;
        TEST(pr.loadPlugin(p.c_str()), "Failed to load plugin into PluginRegister.");
        
        ConnectionGraph cg;
        pr.registerModules(&cg);

        TEST((cg.addModule("SDK_EX.EX_MOD") >= 0), "Failed to add expected module to connection graph."); // notice, prefixed and uppercase
    }

    return 0;
}
