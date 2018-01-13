#include "pluginloader.hpp"

#include <iostream>

typedef const PluginData*(*GetPluginDataPointer)(void);

const char* c_pluginEntryPoint = "getPluginData";

namespace PhasePhckr {

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
        std::cerr << "PluginLoader::loadPlugin '" << filename << "' not found or invalid." << std::endl;
        return nullptr;
    }
    else {
        state = LoadState::loaded;
        InitializerType initializer = LibraryInitializeFunction(lib, c_pluginEntryPoint);
        if (initializer == NULL) {
            std::cerr << "PluginLoader::loadPlugin '" << filename << "' is missing function '" << c_pluginEntryPoint << "'." << std::endl;
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
    std::cerr << "PluginLoader::loadPlugin uknown error" << std::endl;
    return nullptr;
}

const PluginData* PluginLoader::getData() const {
    return pluginData;
}

}
