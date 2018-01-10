#include "plugin.hpp"

static struct ExPluginData : public PluginData {
    const std::string name = "example";
    virtual void listModules(std::map<std::string, std::function<Module*(void)>> modules) const {
        // ... do stuff
    }
} pluginData;

EXPORT const PluginData* getPluginData() {
    return &pluginData;
}
