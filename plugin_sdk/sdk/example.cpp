#include "plugin_api.hpp"

static const char* c_name = "sdk_ex";

class ExPluginModule : public ModuleCRTP<ExPluginModule> {
public:
    ExPluginModule() {
        inputs.push_back(Pad("in"));
        outputs.push_back(Pad("out"));
    }
    virtual void processSample(int sample) override {
        outputs[0].values[sample] = inputs[0].values[sample];
    }
    virtual std::string docString() const override {
        return "An example module from a plugin!";
    }
    static Module* factory() { return new ExPluginModule(); }
};

static struct ExPluginData : public PluginData {
    virtual const char* getName() const {
        return c_name;
    }
    virtual void enumerateFactories(ModuleFactoryMap& modules) const {
        modules["ex_mod"] = ExPluginModule::factory;
    }
} pluginData;

EXPORT const PluginData* getPluginData() {
    return &pluginData;
}
