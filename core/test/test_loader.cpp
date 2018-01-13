#include <iostream>

#include "pluginloader.hpp"
#include "pluginsregister.hpp"
#include "connectiongraph.hpp"

int main(int argc, const char* argv[])
{
    std::string p = BuildDylibName("plugin_example"); // see example.cpp

    {
        std::cout << std::endl << "TEST PhasePhckr::PluginLoader" << std::endl;

        PhasePhckr::PluginLoader ex(p.c_str());
        auto d = ex.getData();

        if (!d) {
            std::cerr << "e: " << p << " failed to load" << std::endl;
            return -1;
        }
        std::cout << d->getName() << " (" << p << ")" << " loaded" << std::endl;

        ModuleFactoryMap m;

        d->enumerateFactories(m);

        if (!m.size()) {
            std::cerr << "e: No factories enumerated!" << std::endl;
            return -2;
        }

        for (const auto& kv : m) {
            auto m = kv.second();
            std::cout << kv.first << " - " << m->docString() << std::endl;
        }

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
        std::cout << std::endl << "TEST PhasePhckr::PluginsRegister" << std::endl;

        PhasePhckr::PluginsRegister pr;
        pr.loadPlugin(p.c_str());
        
        ConnectionGraph cg;
        pr.registerModules(&cg);

        auto m = cg.addModule("EXAMPLE/EX"); // notice, prefixed and uppercase
    }

    return 0;
}
