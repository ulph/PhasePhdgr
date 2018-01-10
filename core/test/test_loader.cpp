#include <iostream>

#include "plugin.hpp"

int main(int argc, const char* argv[])
{
    const auto p = "plugin_example";

    PluginLoader ex(p);
    auto d = ex.getData();

    if (!d) {
        std::cerr << "e: " << p << " failed to load" << std::endl;
        return -1;
    }
    std::cout << d->getName() << " (" << p << ")" << " loaded" <<std::endl;

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
}