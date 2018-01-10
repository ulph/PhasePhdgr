#pragma once

#include <map>
#include <string>
#include <functional>

#include "module.hpp" // TODO, split out API/SDK bits from the module class or rethink

#define ISWINDOWS _WIN32
#if ISWINDOWS
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C"
#endif //ISWINDOWS

typedef std::map<std::string, std::function<Module*(void)>> ModuleFactoryMap;

struct PluginData {
    virtual const char* getName() const = 0;
    virtual void enumerateFactories(ModuleFactoryMap& modules) const = 0;
};
