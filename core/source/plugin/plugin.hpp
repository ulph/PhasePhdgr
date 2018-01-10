#pragma once

#include <set>
#include <map>
#include <string>
#include <functional>

#include "module.hpp"

#define ISWINDOWS _WIN32
#if ISWINDOWS
#include <Windows.h>
#define EXPORT extern "C" __declspec(dllexport)
#define LibraryType HMODULE
#define InitializerType FARPROC
#define LibraryLoadFunction LoadLibrary
#define LibraryInitializeFunction GetProcAddress
#define LibraryUnloadFunction FreeLibrary
#else
#define EXPORT extern "C"
#define LibraryType void*
#define InitializerType void*
#define LibraryLoadFunction dlopen
#define LibraryInitializeFunction dlsym
#define LibraryUnloadFunction dlclose
#endif //ISWINDOWS

struct PluginData {
    virtual const char* getName() const = 0;
    virtual void enumerateFactories( std::map<std::string, std::function<Module*(void)>> modules) const = 0;
};

typedef const PluginData*(*GetPluginDataPointer)(void);

class PluginLoader {
public:
    const std::string filename;
    PluginLoader(const char* filename);
    virtual ~PluginLoader();
    const PluginData* getData() const;
private:
    enum class LoadState {
        notLoaded,
        loaded,
        initialized,
        ready
    };
    LibraryType lib;
    LoadState state = LoadState::notLoaded;
    const PluginData* pluginData = nullptr;
    void unloadPlugin();
    const PluginData* loadPlugin(const char* filename);
};
