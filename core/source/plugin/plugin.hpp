#pragma once

#include <set>
#include <map>
#include <string>

#include "module.hpp"

#define EXPORT extern "C"

#define ISWINDOWS _WIN32
#if ISWINDOWS
#include <Windows.h>
#define LibraryType HMODULE
#define InitializerType FARPROC
#define LibraryLoadFunction LoadLibrary
#define LibraryInitializeFunction GetProcAddress
#define LibraryUnloadFunction FreeLibrary
#else
#define LibraryType void*
#define InitializerType void*
#define LibraryLoadFunction dlopen
#define LibraryInitializeFunction dlsym
#define LibraryUnloadFunction dlclose
#endif //ISWINDOWS

const char* c_pluginEntryPoint = "getPluginData";

struct PluginData {
    const std::string name = "...";
    virtual void registerPluginModules(ConnectionGraph &cg) = 0;
};

typedef const PluginData*(*GetPluginDataPointer)(void);

class PluginLoader {
public:
    const std::string filename;
    PluginLoader(const char* filename);
    virtual ~PluginLoader();
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
