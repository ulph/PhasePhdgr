#pragma once

#include <map>
#include <map>
#include <string>

#include "plugin_api.hpp" // TODO, should not be here

class ConnectionGraph;

namespace PhasePhckr {
    class PluginLoader;

    class PluginsRegister {
        std::map<std::string, PluginLoader*> plugins; // TODO, unique pointers
        ModuleFactoryMap modules;
    public:
        PluginsRegister();
        virtual ~PluginsRegister();
        void registerModules(ConnectionGraph* cg);
        bool loadPlugin(const char* pluginName);
    };
}
