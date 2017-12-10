#ifndef PLUGINPROCESSORFX_H_INCLUDED
#define PLUGINPROCESSORFX_H_INCLUDED

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

class PhasePhckrProcessorFX  : public AudioProcessor
{

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhasePhckrProcessorFX)
    void setEffectChain(const PhasePhckr::PatchDescriptor &p);
    PhasePhckr::Effect* effect;

    TimeSliceThread fileWatchThread;
    DirectoryContentsList componentDirectoryWatcher;
    StupidFileChangeListener componentFilesListener;
    
    PhasePhckr::PatchDescriptor effectChain;
    int activeEffectHandle;

    void updateComponentRegister(const DirectoryContentsList* d);
    PhasePhckr::ComponentRegister componentRegister;
    int componentRegisterHandle;

    float barPosition = 0.f;

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
    void setPatch(const PatchDescriptor& patch);

    PhasePhckrParameters parameters;

};

#endif  // PLUGINPROCESSORFX_H_INCLUDED
