#include "pluginsregister.hpp"
#include "pluginloader.hpp"
#include "connectiongraph.hpp"

#include "phasephckr/design.hpp"

#include <iostream>
#include <algorithm>

#include <string>

namespace PhasePhckr {

    bool pluginNameIsValid(const std::string& n) {
        return pathedModuleTypeIsValid(n);
    }

    bool pluginModuleNameIsValid(const std::string& n) {
        return pathedModuleTypeIsValid(n);
    }

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
            std::cerr << "PluginsRegister::loadPlugin error '" << pluginFileName << "' Invalid name '" << pluginName << "'" << std::endl;
            delete p;
            return false;
        }

        if (plugins.count(pluginName)) {
            std::cerr << "PluginsRegister::loadPlugin error '" << pluginFileName << "' Duplicate name '" << pluginName << "'" << " was defined in '" << plugins.at(pluginName)->filename << "'" << std::endl;
            delete p;
            return false;
        }

        plugins[pluginName] = p;

        ModuleFactoryMap newModules;
        d->enumerateFactories(newModules);

        for (const auto& kv : newModules) {
            std::string n = pluginName + "/";
            std::string m = kv.first;
            std::transform(m.begin(), m.end(), m.begin(), ::toupper);
            if (!moduleNameIsValid(m)) {
                std::cerr << "PluginsRegister::loadPlugin error '" << pluginFileName << "' Invalid module name '" << n << "'" << std::endl;
                continue;
            }
            n += m;
            if (!pluginModuleNameIsValid(n)) {
                std::cerr << "PluginsRegister::loadPlugin error '" << pluginFileName << "' Invalid module name '" << n << "'" << std::endl;
                continue;
            }
            if (modules.count(n)) {
                std::cerr << "PluginsRegister::loadPlugin error '" << pluginFileName << "' Duplicate module name '" << n << "'" << std::endl;
                continue;
            }
            std::cout << "PluginsRegister::loadPlugin loaded '" << n << "' ('" << pluginFileName << "')" << std::endl;
            modules[n] = kv.second;
        }

        std::cout << "PluginsRegister::loadPlugin ok '" << pluginName << "' ('" << pluginFileName << "')" << std::endl;

        return true;
    }
}