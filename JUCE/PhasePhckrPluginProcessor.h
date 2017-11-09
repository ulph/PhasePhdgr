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

using namespace std;
using namespace PhasePhckrFileStuff;

class PhasePhckrAudioProcessor  : public AudioProcessor
{

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhasePhckrAudioProcessor)
    void setVoiceChain(const PhasePhckr::PatchDescriptor &p);
    void setEffectChain(const PhasePhckr::PatchDescriptor &p);
    PhasePhckr::Synth* synth;

    TimeSliceThread fileWatchThread;
    DirectoryContentsList componentDirectoryWatcher;
    StupidFileChangeListener componentFilesListener;

    PhasePhckr::PatchDescriptor voiceChain;
    int activeVoiceHandle;

    PhasePhckr::PatchDescriptor effectChain;
    int activeEffectHandle;

    void updateComponentRegister(const DirectoryContentsList* d);
    PhasePhckr::ComponentRegister componentRegister;
    int componentRegisterHandle;

    std::atomic_flag synthUpdateLock = ATOMIC_FLAG_INIT;

public:
    PhasePhckrAudioProcessor();
    ~PhasePhckrAudioProcessor();

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
    PatchDescriptor getPatch(ParameterType type);
    PresetDescriptor getPreset();
    void setPreset(const PresetDescriptor& preset);

    PhasePhckrParameters parameters;

};

#endif  // PLUGINPROCESSOR_H_INCLUDED
