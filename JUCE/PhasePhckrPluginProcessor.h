#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include <list>

#include <phasephckr.hpp>
#include "JuceHeader.h"

#include "PhasePhckrGrid.h"
#include "Utils.hpp"
#include "PatchEditor.hpp"
#include "DirectoryWatcher.hpp"

#include "PhasePhckrParameter.hpp"

#include "PhasePhckrPluginCommon.h"


using namespace std;
using namespace PhasePhckrFileStuff;

class PhasePhckrProcessor  : public AudioProcessor
{

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhasePhckrProcessor)
    void setVoiceChain(const PhasePhckr::PatchDescriptor &p);
    void setEffectChain(const PhasePhckr::PatchDescriptor &p);
    PhasePhckr::Synth* synth;

    ProcessorFileThings fileThings;

    PhasePhckr::PatchDescriptor voiceChain;
    int activeVoiceHandle;

    PhasePhckr::PatchDescriptor effectChain;
    int activeEffectHandle;

    PhasePhckr::ComponentRegister componentRegister;
    int componentRegisterHandle;

    float barPosition = 0.f;

    simple_lock synthUpdateLock;

    int lastBlockSize = 0;
    int carryOverSamples = 0;
    float* carryOverBlockBuffer[2] = { nullptr };

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

    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    const PhasePhckr::Synth* getSynth() const;

    SubValue<PatchDescriptor> subVoiceChain;
    SubValue<PatchDescriptor> subEffectChain;
    SubValue<PhasePhckr::ComponentRegister> subComponentRegister;

    void broadcastPatch();
    vector<PresetParameterDescriptor> getPresetParameters();
    vector<PatchParameterDescriptor> getParameters(SynthGraphType type);
    PatchDescriptor getPatch(SynthGraphType type, bool extractParameters=false);
    PresetDescriptor getPreset();
    void setPreset(const PresetDescriptor& preset);
    void setPatch(SynthGraphType type, const PatchDescriptor& patch);

    void setComponentRegister(const ComponentRegister& cr);

    PhasePhckrParameters parameters;

    void updateLayout(SynthGraphType type, const string &component, const map<string, ModulePosition> &layout);

};

#endif  // PLUGINPROCESSOR_H_INCLUDED
