#include "plugin.hpp"

struct ExPluginData : public PluginData {
    const std::string name = "example";
    virtual void registerPluginModules(ConnectionGraph &cg) {

    }
};

static const auto pluginData = ExPluginData();

EXPORT const PluginData* getPluginData() {
    return &pluginData;
}
