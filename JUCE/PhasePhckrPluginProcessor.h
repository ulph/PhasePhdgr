#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "JuceLibraryCode/JuceHeader.h"
#include "phasephckr.h" // for now
#include "design_json.hpp"
#include "PhasePhckrGrid.h"
#include "components.hpp"
#include "Utils.hpp"
#include <list>
#include "GraphEditor.hpp"

using namespace std;

class PhasePhckrParameter : public AudioParameterFloat {
    int idx;
public:
    PhasePhckrParameter(int idx)
        : AudioParameterFloat(
            to_string(idx),
            to_string(idx / 8) + "_" + to_string(idx % 8),
            0.0f,
            1.0f,
            0.0f
            )
        , idx(idx)
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
    PhasePhckr::PatchDescriptor voiceChain;
    PhasePhckr::PatchDescriptor effectChain;
    int activeVoiceHandle;
    int activeEffectHandle;
    void applyVoiceChain();
    void applyEffectChain();

    std::atomic_flag synthUpdateLock = ATOMIC_FLAG_INIT;

public:
    PhasePhckrAudioProcessor();
    ~PhasePhckrAudioProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

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

    PhasePhckr::ComponentRegister componentRegister;
    SubPatch activeVoice;
    SubPatch activeEffect;
    void broadcastPatch(){
        // editor should call this once after construction
        activeVoice.set(activeVoiceHandle, voiceChain);
        activeEffect.set(activeEffectHandle, effectChain);
    }

    vector<AudioParameterFloat *> floatParameters;
    vector<AudioParameterBool *> boolParameters;

};

#endif  // PLUGINPROCESSOR_H_INCLUDED
