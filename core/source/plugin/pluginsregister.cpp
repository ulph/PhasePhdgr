#include "pluginsregister.hpp"
#include "pluginloader.hpp"
#include "connectiongraph.hpp"

#include "phasephdgr/design.hpp"

#include <iostream>
#include <algorithm>

#include <string>

namespace PhasePhdgr {

    bool pluginNameIsValid(const std::string& n) {
        return typeIsValid(n, true); // same as a scope type to allow building prefixes
    }

    bool pluginModuleNameIsValid(const std::string& n) {
        return typeIsValid(n, true);
    }

    PluginsRegister::PluginsRegister() {
    }

    PluginsRegister::~PluginsRegister() {
        for (auto kv : plugins) delete kv.second;
    }

    void PluginsRegister::registerModules(ConnectionGraph* cg) const {
        for (const auto& kv : modules) {
            cg->registerModule(kv.first, kv.second);
        }
    }

    bool PluginsRegister::loadPlugin(const char* pluginFileName) {
        auto p = new PluginLoader(pluginFileName);
        auto d = p->getData();

        if (d == nullptr) {
            std::cerr << "PluginsRegister::loadPlugin error '" << pluginFileName << "'" << std::endl;
            delete p;
            return false;
        }

        std::string pluginName = d->getName();
        std::transform(pluginName.begin(), pluginName.end(), pluginName.begin(), ::toupper);

        if (!pluginNameIsValid(pluginName)) {
            std::cerr << "PluginsRegister::loadPlugin error '" << pluginFileName << "' Invalid plugin basename '" << pluginName << "'" << std::endl;
            delete p;
            return false;
        }

        if (plugins.count(pluginName)) {
            std::cerr << "PluginsRegister::loadPlugin error '" << pluginFileName << "' Duplicate plugin basename '" << pluginName << "'" << " was defined in '" << plugins.at(pluginName)->filename << "'" << std::endl;
            delete p;
            return false;
        }

        plugins[pluginName] = p;

        ModuleFactoryMap newModules;
        d->enumerateFactories(newModules);

        for (const auto& kv : newModules) {
            std::string n = pluginName + scopeSeparator;
            std::string m = kv.first;
            std::transform(m.begin(), m.end(), m.begin(), ::toupper);
            if (!typeIsValid(m, false)) {
                std::cerr << "PluginsRegister::loadPlugin error '" << pluginFileName << "' Invalid module type '" << n << "'" << std::endl;
                continue;
            }
            n += m;
            if (!pluginModuleNameIsValid(n)) {
                std::cerr << "PluginsRegister::loadPlugin error '" << pluginFileName << "' Invalid plugin module type '" << n << "'" << std::endl;
                continue;
            }
            if (modules.count(n)) {
                std::cerr << "PluginsRegister::loadPlugin error '" << pluginFileName << "' Duplicate module type '" << n << "'" << std::endl;
                continue;
            }
            std::cout << "PluginsRegister::loadPlugin loaded '" << n << "' ('" << pluginFileName << "')" << std::endl;
            modules[n] = kv.second;
        }

        std::cout << "PluginsRegister::loadPlugin ok '" << pluginName << "' ('" << pluginFileName << "')" << std::endl;

        return true;
    }
}
