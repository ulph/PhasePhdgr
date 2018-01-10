#include "plugin.hpp"

PluginLoader::PluginLoader(const char* filename)
    : filename(filename)
{
    pluginData = loadPlugin(filename);
}

PluginLoader::~PluginLoader() {
    unloadPlugin();
}

void PluginLoader::unloadPlugin() {
    if (state != LoadState::notLoaded) {
        LibraryUnloadFunction(lib);
    }
}

const PluginData* PluginLoader::loadPlugin(const char* filename) {
    GetPluginDataPointer getPluginData = nullptr;
    lib = LibraryLoadFunction(filename);
    if (lib == NULL) {
        return nullptr;
    }
    else {
        state = LoadState::loaded;
        InitializerType initializer = LibraryInitializeFunction(lib, c_pluginEntryPoint);
        if (initializer == NULL) {
            return nullptr;
        }
        else {
            state = LoadState::initialized;
            getPluginData = (GetPluginDataPointer)initializer;
            auto* d = getPluginData();
            state = LoadState::ready;
            return d;
        }
    }
    return nullptr;
}

const PluginData* PluginLoader::getData() const {
    return pluginData;
}