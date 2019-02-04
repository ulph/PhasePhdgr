#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include <list>

#include <phasephckr.hpp>
#include "JuceHeader.h"

#include "Utils.hpp"
#include "PatchEditor.hpp"
#include "DirectoryWatcher.hpp"

#include "Parameters.hpp"

#include "PluginCommon.h"

#include <functional>

using namespace std;
using namespace PhasePhckrFileStuff;

class PhasePhckrProcessorBase : public AudioProcessor {
public:
    struct InstanceSpecificPeristantState {
        // slightly less than 720p
        int width = 1000;
        int height = 700;
    };

    PhasePhckrProcessorBase(const BusesProperties& ioConfig)
        : AudioProcessor(ioConfig)
        , componentLoader(subComponentRegister)
    {}

protected:
    ComponentFileLoader componentLoader;

    PhasePhckr::PresetSettings activeSettings;
    int activeSettingsHandle;

    PhasePhckr::ComponentRegister componentRegister;
    int componentRegisterHandle;

    InstanceSpecificPeristantState extra;

    simple_lock synthUpdateLock;

public:

    void getStateInformation(MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    SubValue<PresetSettings> subSettings;
    SubValue<PhasePhckr::ComponentRegister> subComponentRegister;

    void setPreset(const PresetDescriptor& preset);
    PresetDescriptor getPreset();

    virtual PatchDescriptor getPatch(SynthGraphType type, bool extractParameters = false) = 0;
    virtual void setPatch(SynthGraphType type, const PatchDescriptor& patch) = 0;
    virtual void setSettings(const PhasePhckr::PresetSettings &settings) = 0;

    Parameters parameters;

};

class PhasePhckrProcessor: public PhasePhckrProcessorBase
{

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhasePhckrProcessor)
    void setVoiceChain(const PhasePhckr::PatchDescriptor &p);
    void setEffectChain(const PhasePhckr::PatchDescriptor &p);
    PhasePhckr::Synth* synth;
    PhasePhckr::Effect* effect;

    PhasePhckr::PatchDescriptor voiceChain;
    int activeVoiceHandle;

    PhasePhckr::PatchDescriptor effectChain;
    int activeEffectHandle;

    GeneratingBufferingProcessor bufferingProcessor;

    size_t voiceHash = 0;
    size_t effectHash = 0;

    vector<PPMidiMessage> midiMessageQueue;

public:
    PhasePhckrProcessor();
    ~PhasePhckrProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;

    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    const PhasePhckr::Synth* getSynth() const;
    const PhasePhckr::Effect* getEffect() const;

    SubValue<PatchDescriptor> subVoiceChain;
    SubValue<PatchDescriptor> subEffectChain;

    void broadcastPatch();
    vector<PresetParameterDescriptor> getPresetParameters();
    vector<PatchParameterDescriptor> getParameters(SynthGraphType type);
    PatchDescriptor getPatch(SynthGraphType type, bool extractParameters=false) override;

    void setSettings(const PhasePhckr::PresetSettings &settings) override;
    void setPatch(SynthGraphType type, const PatchDescriptor& patch) override;

    void setComponentRegister(const ComponentRegister& cr);

    void updateLayout(SynthGraphType type, const string &component, const map<string, ModulePosition> &layout);

    PPLookAndFeel lookAndFeel;

    void forceStateBump();

    SDKExtensionManager sdkExtensionManager; // no race, as only write is on constructor of processor

};

#endif  // PLUGINPROCESSOR_H_INCLUDED
