#include <iostream>

#include "plugin.hpp"

int main(int argc, const char* argv[])
{
    const auto p = "plugin_example";

    PluginLoader ex(p);
    auto d = ex.getData();

    if (!d) {
        std::cout << p << " failed to load" << std::endl;
        return -1;
    }
    std::cout << d->getName() << " (" << p << ")" << " loaded" <<std::endl;
}