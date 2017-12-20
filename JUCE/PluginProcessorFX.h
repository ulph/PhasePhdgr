#ifndef PLUGINPROCESSORFX_H_INCLUDED
#define PLUGINPROCESSORFX_H_INCLUDED

#include <list>

#include <functional>

#include <phasephckr.hpp>
#include "JuceHeader.h"

#include "PPGrid.h"
#include "Utils.hpp"
#include "PatchEditor.hpp"

#include "Parameter.hpp"

#include "PluginCommon.h"

using namespace std;

class PhasePhckrProcessorFX  : public AudioProcessor
{

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhasePhckrProcessorFX)
    void setEffectChain(const PhasePhckr::PatchDescriptor &p);
    PhasePhckr::Effect* effect;

    ComponentFileLoader componentLoader;
    
    PhasePhckr::PatchDescriptor effectChain;
    int activeEffectHandle;

    PhasePhckr::ComponentRegister componentRegister;
    int componentRegisterHandle;

    InputBufferingProcessor bufferingProcessor;

    size_t effectHash = 0;

    simple_lock synthUpdateLock;

public:
    PhasePhckrProcessorFX();
    ~PhasePhckrProcessorFX();

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

    const Effect* getEffect() const;

    SubValue<PatchDescriptor> subEffectChain;
    SubValue<PhasePhckr::ComponentRegister> subComponentRegister;

    void broadcastPatch();
    PatchDescriptor getPatch();
    PresetDescriptor getPreset();
    void setPreset(const PresetDescriptor& preset);
    void setPatch(const PatchDescriptor& patch);

    void setComponentRegister(const ComponentRegister& cr);

    Parameters parameters;

    void updateLayout(const string &component, const map<string, ModulePosition> &layout);

};

#endif  // PLUGINPROCESSORFX_H_INCLUDED
