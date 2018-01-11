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
#else
#include <dlfcn.h>
#define LibraryType void*
#define InitializerType void*
#define LibraryLoadFunction(arg) dlopen(arg, RTLD_LAZY)
#define LibraryInitializeFunction dlsym
#define LibraryUnloadFunction dlclose
#endif //ISWINDOWS

#if ISWINDOWS
#define PLUGINPREFIX ""
#define DYLIBEXT ".dll"
#elif __APPLE__
#define PLUGINPREFIX "lib"
#define DYLIBEXT ".dylib"
#else
#define PLUGINPREFIX "lib"
#define DYLIBEXT ".so"
#endif

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
