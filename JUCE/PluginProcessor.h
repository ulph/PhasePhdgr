#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include <list>

#include <phasephdgr.hpp>

#include "Utils.hpp"
#include "PatchEditor.hpp"
#include "DirectoryWatcher.hpp"

#include "Parameters.hpp"

#include "PluginCommon.h"

#include <functional>

#include <juce_audio_processors/juce_audio_processors.h>

using namespace std;
using namespace PhasePhdgrFileStuff;
using namespace juce;

class PhasePhdgrProcessorBase : public AudioProcessor {
public:
    struct InstanceSpecificPeristantState {
        // slightly less than 720p
        int width = 1000;
        int height = 700;
    };

    PhasePhdgrProcessorBase(const BusesProperties& ioConfig)
        : AudioProcessor(ioConfig)
        , componentLoader(subComponentRegister)
    {}

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool hasEditor() const override;
    const String getName() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const String getProgramName(int index) override;
    void changeProgramName(int index, const String& newName) override;

    bool isSynth() const {
        return getProcessor(SynthGraphType::VOICE) != nullptr;         
    }

protected:
    ComponentFileLoader componentLoader;

    PhasePhdgr::PresetSettings activeSettings;
    int activeSettingsHandle;

    PhasePhdgr::ComponentRegister componentRegister;
    int componentRegisterHandle;

    InstanceSpecificPeristantState extra;

    simple_lock synthUpdateLock;

    struct processor_bundle {
        PhasePhdgr::Base* processor = nullptr;
        PhasePhdgr::PatchDescriptor patch;
        size_t hash = 0;
        int handle = std::numeric_limits<int>::max();
        SubValue<PatchDescriptor> propagator;
        void broadcast() const {
            assert(processor);
            propagator.set(handle, patch);
        }
        void unsubscribe() {
            propagator.unsubscribe(handle);
        }
    };

    std::map<SynthGraphType, processor_bundle> bundles;

    void initialize();

    void destroy();

public:

    void getStateInformation(MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    SubValue<PresetSettings> subSettings;
    SubValue<PhasePhdgr::ComponentRegister> subComponentRegister;

    SubValue<PatchDescriptor> nullPatchPropagator;

    void setPreset(const PresetDescriptor& preset);
    PresetDescriptor getPreset();

    virtual void setSettings(const PhasePhdgr::PresetSettings &settings);

    Parameters parameters;

    vector<PresetParameterDescriptor> getPresetParameters();
    vector<PatchParameterDescriptor> getParameters(SynthGraphType type);

    void forceStateBump();

    void setComponentRegister(const ComponentRegister& cr);

    SDKExtensionManager sdkExtensionManager; // no race, as only write is on constructor of processor

    void broadcastPatch();
    PatchDescriptor getPatch(SynthGraphType type, bool extractParameters = false);
    void setPatch(SynthGraphType type, const PatchDescriptor& patch, bool cleanLocalComponentDupe=false);

    SubValue<PatchDescriptor> &getPropagator(SynthGraphType type);

    void updateLayout(SynthGraphType type, const string &component, const map<string, ModulePosition> &layout);

    PPLookAndFeel lookAndFeel;

    const PhasePhdgr::Base* getProcessor(SynthGraphType type) const;

};

class PhasePhdgrProcessor: public PhasePhdgrProcessorBase
{

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhasePhdgrProcessor)
    PhasePhdgr::Synth* synth;
    PhasePhdgr::Effect* effect;

    GeneratingBufferingProcessor bufferingProcessor;

    vector<PPMidiMessage> midiMessageQueue;

public:
    PhasePhdgrProcessor();
    ~PhasePhdgrProcessor();

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;

    AudioProcessorEditor* createEditor() override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    const String getName() const override;
};


class PhasePhdgrProcessorFx : public PhasePhdgrProcessorBase
{

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhasePhdgrProcessorFx)
    PhasePhdgr::Effect* effect;

    InputBufferingProcessor bufferingProcessor;

public:
    PhasePhdgrProcessorFx();
    ~PhasePhdgrProcessorFx();

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(AudioSampleBuffer&, MidiBuffer&) override;

    AudioProcessorEditor* createEditor() override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    const String getName() const override;
};


#endif  // PLUGINPROCESSOR_H_INCLUDED
