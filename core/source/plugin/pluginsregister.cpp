#include "pluginsregister.hpp"
#include "pluginloader.hpp"
#include "connectiongraph.hpp"

#include <iostream>
#include <algorithm>

bool pluginNameIsValid(const char* n) {
    return true;
}

bool pluginModuleNameIsValid(const char* n) {
    return true;
}

namespace PhasePhckr {
    PluginsRegister::PluginsRegister() {
    }

    PluginsRegister::~PluginsRegister() {
        for (auto kv : plugins) delete kv.second;
    }

    void PluginsRegister::registerModules(ConnectionGraph* cg) {
        for (const auto& kv : modules) {
            cg->registerModule(kv.first, kv.second);
        }
    }

    bool PluginsRegister::loadPlugin(const char* pluginName) {
        auto p = new PluginLoader(pluginName);
        auto d = p->getData();

        if (d == nullptr) {
            std::cerr << "PluginsRegister::loadPlugin error '" << pluginName << "'" << std::endl;
            delete p;
            return false;
        }

        if (!pluginNameIsValid(d->getName())) {
            std::cerr << "PluginsRegister::loadPlugin error '" << pluginName << "' Invalid name '" << d->getName() << "'" << std::endl;
            delete p;
            return false;
        }

        if (plugins.count(d->getName())) {
            std::cerr << "PluginsRegister::loadPlugin error '" << pluginName << "' Duplicate name '" << d->getName() << "'" << " was defined in '" << plugins.at(d->getName())->filename << "'" << std::endl;
            delete p;
            return false;
        }

        plugins[d->getName()] = p;

        ModuleFactoryMap newModules;
        d->enumerateFactories(newModules);

        for (const auto& kv : newModules) {
            std::string n = d->getName();
            n += "/";
            n += kv.first;
            std::transform(n.begin(), n.end(), n.begin(), ::toupper);
            if (!pluginModuleNameIsValid(n.c_str())) {
                std::cerr << "PluginsRegister::loadPlugin error '" << pluginName << "' Invalid module name '" << n << "'" << std::endl;
                continue;
            }
            if (modules.count(n)) {
                std::cerr << "PluginsRegister::loadPlugin error '" << pluginName << "' Duplicate module name '" << n << "'" << std::endl;
                continue;
            }
            modules[n] = kv.second;
        }

        std::cout << "PluginsRegister::loadPlugin ok '" << d->getName() << "' ('" << pluginName << "')" << std::endl;

        return true;
    }
}