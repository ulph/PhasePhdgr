#pragma once

#include <map>
#include <string>

#include "module.hpp"

#define ISWINDOWS _WIN32
#if ISWINDOWS
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C"
#endif //ISWINDOWS

typedef std::map<std::string, Module* (*)()> ModuleFactoryMap; // move elsewhere

struct PluginData {
    virtual const char* getName() const = 0;
    virtual void enumerateFactories(ModuleFactoryMap& modules) const = 0;
};
