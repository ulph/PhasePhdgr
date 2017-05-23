#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "JuceHeader.h"

#include "phasephckr.h" // for now
#include "design_json.hpp"
#include "PhasePhckrGrid.h"
#include "components.hpp"
#include "Utils.hpp"
#include <list>
#include "PatchEditor.hpp"
#include "DirectoryWatcher.hpp"

using namespace std;
using namespace PhasePhckrFileStuff;

class PhasePhckrParameter : public AudioParameterFloat {
public:
    PhasePhckrParameter(int idx)
        : AudioParameterFloat(
            to_string(idx),
            to_string(idx / 8) + "_" + to_string(idx % 8),
            0.0f,
            1.0f,
            0.0f
            )
    {
    }
};

class PhasePhckrAudioProcessor  : public AudioProcessor
{

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhasePhckrAudioProcessor)
    void setVoiceChain(const PhasePhckr::PatchDescriptor &p) {
        voiceChain = p;
        applyVoiceChain();
    }
    void setEffectChain(const PhasePhckr::PatchDescriptor &p) {
        effectChain = p;
        applyEffectChain();
    }
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

    void applyVoiceChain();
    void applyEffectChain();

    std::atomic_flag synthUpdateLock = ATOMIC_FLAG_INIT;

    void updateParameters(bool newVoiceChain, bool newEffectChain);
    vector<AudioParameterFloat *> floatParameters;
    enum ApiType {
        VOICE,
        EFFECT
    };
    map<int, pair<ApiType, int>> parameterRouting;
    map<string, int> effectParameters;
    map<string, int> voiceParameters;

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

    SubValue<PatchDescriptor> activeVoice;
    SubValue<PatchDescriptor> activeEffect;
    SubValue<PhasePhckr::ComponentRegister> subComponentRegister;

    void broadcastPatch(){
        // editor should call this once after construction
        subComponentRegister.set(componentRegisterHandle, componentRegister);
        activeVoice.set(activeVoiceHandle, voiceChain);
        activeEffect.set(activeEffectHandle, effectChain);
    }

};

#endif  // PLUGINPROCESSOR_H_INCLUDED
