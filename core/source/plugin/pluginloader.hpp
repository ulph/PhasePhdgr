#pragma once

#include <set>
#include <map>
#include <string>
#include <functional>

#include "plugin_api.hpp"

#define ISWINDOWS _WIN32
#if ISWINDOWS
#include <Windows.h>
#define LibraryType HMODULE
#define InitializerType FARPROC
#define LibraryLoadFunction LoadLibrary
#define LibraryInitializeFunction GetProcAddress
#define LibraryUnloadFunction FreeLibrary
#define DYLIBEXT ".dll"
#else
#define LibraryType void*
#define InitializerType void*
#define LibraryLoadFunction dlopen
#define LibraryInitializeFunction dlsym
#define LibraryUnloadFunction dlclose
#define DYLIBEXT ".so"
#endif //ISWINDOWS

#define PLUGINSUFFIX ".ppp"

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
