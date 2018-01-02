#pragma once

#include <phasephckr.hpp>
#include "JuceHeader.h"

#include "DirectoryWatcher.hpp"
#include "FileIO.hpp"
#include "Utils.hpp"

#include "ParameterKnob.hpp"
#include "PPGrid.h"

#include <functional>

using namespace PhasePhckrFileStuff;
using namespace PhasePhckr;
using namespace std;

void storeState(const PresetDescriptor& preset, MemoryBlock& destData);

void loadState(const void* data, int sizeInBytes, PresetDescriptor& preset);

void handlePlayHead(Effect* effect, AudioPlayHead* playHead, const int blockSize, const float sampleRate, float& barPosition);

class BufferingProcessor {
public:
    enum class Strategy {
        BUFFERING,
        AHEAD // effectively doing underruns, which is ok for a purely generating processor
    };

protected:
    Strategy strategy;

    float barPosition = 0;
    int inputBufferSamples = 0;
    int outputBufferSamples; // set via getLatency() in initializer list
    float* inputBuffer[2] = { nullptr }; // use vectors instead?
    float* outputBuffer[2] = { nullptr };
    AudioSampleBuffer scratchBuffer;

public:
    virtual int getLatency() {
        return strategy == Strategy::BUFFERING ? Effect::internalBlockSize() : 0;
    }

    virtual void process(AudioSampleBuffer& buffer, float sampleRate, Effect* effect, AudioPlayHead* playHead) = 0;

    BufferingProcessor(const Strategy& strategy_ = Strategy::BUFFERING)
        : strategy(strategy_)
        , outputBufferSamples(getLatency())
    {
        inputBuffer[0] = new float[Synth::internalBlockSize()]{ 0.0f };
        inputBuffer[1] = new float[Synth::internalBlockSize()]{ 0.0f };
        outputBuffer[0] = new float[Synth::internalBlockSize()]{ 0.0f };
        outputBuffer[1] = new float[Synth::internalBlockSize()]{ 0.0f };
    }

    virtual ~BufferingProcessor() {
        delete[] inputBuffer[0];
        delete[] inputBuffer[1];
        delete[] outputBuffer[0];
        delete[] outputBuffer[1];
    }
};


class GeneratingBufferingProcessor : public BufferingProcessor {
public:
    GeneratingBufferingProcessor()
        : BufferingProcessor(Strategy::AHEAD)
    {
    }

    virtual void process(AudioSampleBuffer& buffer, float sampleRate, Effect* synth, AudioPlayHead* playHead) {
        const int blockSize = buffer.getNumSamples();
        const int internalBlockSize = Synth::internalBlockSize();

        assert(outputBufferSamples >= 0);
        assert(outputBufferSamples <= internalBlockSize);

        int bufferOffset = (internalBlockSize - outputBufferSamples) % internalBlockSize;

        // samples from last call
        int destinationBufferOffset = 0;
        if (bufferOffset > 0) {
            auto* l = buffer.getWritePointer(0);
            auto* r = buffer.getWritePointer(1);
            int i = 0;
            for (i = 0; i < outputBufferSamples && i < blockSize; ++i) {
                l[i] = outputBuffer[0][bufferOffset + i];
                r[i] = outputBuffer[1][bufferOffset + i];
                outputBuffer[0][bufferOffset + i] = 0.f;
                outputBuffer[1][bufferOffset + i] = 0.f;
            }
            destinationBufferOffset += i;
            outputBufferSamples -= i;
        }

        if (outputBufferSamples > 0) {
            assert(destinationBufferOffset == blockSize);
            return; // no processing until we've emptied the outputBuffer
        }

        assert(outputBufferSamples == 0);

        const int nominalBlockSize = blockSize - destinationBufferOffset;
        const int alignedBlockSize = internalBlockSize * (nominalBlockSize / internalBlockSize);

        assert(alignedBlockSize >= 0);
        assert(alignedBlockSize <= blockSize);

        int carryOverSamples = nominalBlockSize - alignedBlockSize;

        assert((destinationBufferOffset + alignedBlockSize + carryOverSamples) == blockSize);

        // samples, if any, that fits a multiple of Synth::internalBlockSize
        if (alignedBlockSize > 0) {
            handlePlayHead(synth, playHead, alignedBlockSize, sampleRate, barPosition);
            synth->update(buffer.getWritePointer(0, destinationBufferOffset), buffer.getWritePointer(1, destinationBufferOffset), alignedBlockSize, sampleRate);
            destinationBufferOffset += alignedBlockSize;
        }

        // if not all samples fit, calculate a new frame and store
        if (carryOverSamples > 0) {
            handlePlayHead(synth, playHead, internalBlockSize, sampleRate, barPosition);
            synth->update(outputBuffer[0], outputBuffer[1], internalBlockSize, sampleRate);
            auto* l = buffer.getWritePointer(0, destinationBufferOffset);
            auto* r = buffer.getWritePointer(1, destinationBufferOffset);
            int i = 0;
            for (i = 0; i < carryOverSamples; ++i) {
                l[i] = outputBuffer[0][i];
                r[i] = outputBuffer[1][i];
                outputBuffer[0][i] = 0.f;
                outputBuffer[1][i] = 0.f;
            }
            outputBufferSamples = internalBlockSize - carryOverSamples;
            destinationBufferOffset += carryOverSamples;
        }

        assert(destinationBufferOffset == blockSize);
        assert(outputBufferSamples >= 0);
        assert(outputBufferSamples <= internalBlockSize);

    }
};


class InputBufferingProcessor: public BufferingProcessor{
public:
    InputBufferingProcessor() 
        : BufferingProcessor(Strategy::BUFFERING)
    {
    }

    virtual void process(AudioSampleBuffer& buffer, float sampleRate, Effect* effect, AudioPlayHead* playHead) {
        // TODO, merge inputBuffer into scratchBuffer to reduce amount of copying back and forth

        // pre checks
        assert(buffer.getNumChannels() >= 2);

        if (scratchBuffer.getNumChannels() != buffer.getNumChannels() || scratchBuffer.getNumSamples() != 2 * buffer.getNumSamples()) {
            scratchBuffer.setSize(buffer.getNumChannels(), 2 * buffer.getNumSamples(), true, true, true);
        }

        const int blockSize = buffer.getNumSamples();
        const int internalBlockSize = Effect::internalBlockSize();

        // buffer input and process, bump to scratchBuffer
        int i = 0;
        for (i = 0; i < internalBlockSize - inputBufferSamples && i < blockSize; i++) {
            inputBuffer[0][inputBufferSamples + i] = buffer.getSample(0, i);
            inputBuffer[1][inputBufferSamples + i] = buffer.getSample(1, i);
        }
        inputBufferSamples += i;

        auto scratchBufferSize = 0;
        if (inputBufferSamples == internalBlockSize) {
            handlePlayHead(effect, playHead, internalBlockSize, sampleRate, barPosition);
            effect->update(inputBuffer[0], inputBuffer[1], internalBlockSize, sampleRate);
            for (int j = 0; j < internalBlockSize; j++) {
                scratchBuffer.setSample(0, j, inputBuffer[0][j]);
                scratchBuffer.setSample(1, j, inputBuffer[1][j]);
            }
            scratchBufferSize += internalBlockSize;
            inputBufferSamples -= internalBlockSize;

            auto alignedBlockSize = internalBlockSize * ((blockSize - i) / internalBlockSize);
            if (alignedBlockSize) {
                handlePlayHead(effect, playHead, alignedBlockSize, sampleRate, barPosition);
                effect->update(buffer.getWritePointer(0, i), buffer.getWritePointer(1, i), alignedBlockSize, sampleRate);
                scratchBuffer.copyFrom(0, scratchBufferSize, buffer.getReadPointer(0, i), alignedBlockSize);
                scratchBuffer.copyFrom(1, scratchBufferSize, buffer.getReadPointer(1, i), alignedBlockSize);
            }
            scratchBufferSize += alignedBlockSize;

            int toBuffer = blockSize - i - alignedBlockSize;
            for (int j = 0; j < toBuffer && inputBufferSamples + j < internalBlockSize; j++) {
                inputBuffer[0][inputBufferSamples + j] = buffer.getSample(0, i + alignedBlockSize + j);
                inputBuffer[1][inputBufferSamples + j] = buffer.getSample(1, i + alignedBlockSize + j);
            }
            inputBufferSamples += toBuffer;
        }

        // copy samples from outputBuffer and scratchBuffer, bump remainder to outputBuffer
        int toCopy = 0;
        if (outputBufferSamples + scratchBufferSize >= blockSize) {
            int o = 0;
            for (o = 0; o < outputBufferSamples && o < blockSize; o++) {
                buffer.setSample(0, o, outputBuffer[0][o]);
                buffer.setSample(1, o, outputBuffer[1][o]);
            }
            outputBufferSamples -= o;
            for (int j = 0; j < internalBlockSize - o; j++) {
                outputBuffer[0][j] = outputBuffer[0][o + j];
                outputBuffer[1][j] = outputBuffer[1][o + j];
            }

            toCopy = (scratchBufferSize > blockSize - o) ? blockSize - o : scratchBufferSize;
            if (toCopy) {
                buffer.copyFrom(0, o, scratchBuffer.getReadPointer(0, 0), toCopy);
                buffer.copyFrom(1, o, scratchBuffer.getReadPointer(1, 0), toCopy);
            }
        }
        int toBuffer = scratchBufferSize - toCopy;
        for (int j = 0; j < toBuffer && j < outputBufferSamples + internalBlockSize; j++) {
            outputBuffer[0][outputBufferSamples + j] = scratchBuffer.getSample(0, toCopy + j);
            outputBuffer[1][outputBufferSamples + j] = scratchBuffer.getSample(1, toCopy + j);
        }
        outputBufferSamples += toBuffer;

        // post checks
        assert(inputBufferSamples <= internalBlockSize);
        assert(outputBufferSamples <= internalBlockSize);
        assert(inputBufferSamples + outputBufferSamples == internalBlockSize);
    }
};


class ParameterPages : public TabbedComponent, public DragAndDropTarget {
public:
    virtual void currentTabChanged(int newCurrentTabIndex, const String &newCurrentTabName) override {
        getTabContentComponent(newCurrentTabIndex)->resized();
    }
    ParameterPages() : TabbedComponent(TabbedButtonBar::TabsAtBottom) {}
    virtual bool isInterestedInDragSource(const SourceDetails &dragSourceDetails) override {
        return true;
    }
    virtual void itemDropped(const SourceDetails &dragSourceDetails) override {}
    virtual void itemDragMove(const SourceDetails &dragSourceDetails) override {
        TabbedButtonBar& c = getTabbedButtonBar();
        for (int i = 0; i < c.getNumTabs(); ++i) {
            TabBarButton* b = c.getTabButton(i);
            auto pos = dragSourceDetails.localPosition - c.getPosition() - b->getPosition();
            if (b->contains(pos)) {
                setCurrentTabIndex(i);
                return;
            }
        }
    }
};


class ParameterEditor : public Component, public DragAndDropContainer {
private:
    ParameterPages pageTabs;
    vector<PPGrid*> pages;
    int knobCtr = 0;
    const vector<float> rowLayout = { 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f };
public:
    ParameterEditor();
    virtual ~ParameterEditor();
    void addKnob(ParameterKnob* knob);
    void resized() override;
};


#define shutup NotificationType::dontSendNotification


class SettingsEditor : public Component, public ButtonListener {
private:
    PresetSettings settings;
    int subSettingsHandle = -1;
    SubValue<PresetSettings> &subSettings;

    PPGrid grid;

    PPGrid polyGrid;
    TextButton polyphony1;
    TextButton polyphony2;
    TextButton polyphony4;
    TextButton polyphony8;
    TextButton polyphony16;
    TextButton polyphony32;
    TextButton polyphony64;

    PPGrid stealGrid;
    TextButton steal0;
    TextButton steal1;
    TextButton steal2;
    TextButton steal3;
    TextButton steal4;
    TextButton steal5;

    PPGrid reactivateGrid;
    TextButton reactivate0;
    TextButton reactivate1;
    TextButton reactivate2;
    TextButton reactivate3;
    TextButton reactivate4;

    PPGrid legatoGrid;
    TextButton legato0;
    TextButton legato1;
    TextButton legato2;
    TextButton legato3;

    PPGrid activateGrid;
    TextButton activate0;
    TextButton activate1;

    void applySettings(){
        switch(settings.polyphony){
        case 1: polyphony1.setToggleState(true, shutup); break;
        case 2: polyphony2.setToggleState(true, shutup); break;
        case 4: polyphony4.setToggleState(true, shutup); break;
        case 8: polyphony8.setToggleState(true, shutup); break;
        case 16: polyphony16.setToggleState(true, shutup); break;
        case 32: polyphony32.setToggleState(true, shutup); break;
        case 64: polyphony64.setToggleState(true, shutup); break;
        default: PP_NYI; break;
        }

        switch(settings.noteStealPolicy){
        case NoteStealPolicyDoNotSteal: steal0.setToggleState(true, shutup); break;
        case NoteStealPolicyStealOldest: steal1.setToggleState(true, shutup); break;
        case NoteStealPolicyStealLowestRMS: steal2.setToggleState(true, shutup); break;
        case NoteStealPolicyStealIfLower: steal3.setToggleState(true, shutup); break;
        case NoteStealPolicyStealIfHigher: steal4.setToggleState(true, shutup); break;
        case NoteStealPolicyStealYoungest: steal5.setToggleState(true, shutup); break;
        default: PP_NYI; break;
        }

        switch(settings.noteStealPolicy){
        case LegatoModeRetrigger: legato0.setToggleState(true, shutup); break;
        case LegatoModeUpdateVelocity: legato1.setToggleState(true, shutup); break;
        case LegatoModeFreezeVelocity: legato2.setToggleState(true, shutup); break;
        case LegatoModeReleaseVelocity: legato3.setToggleState(true, shutup); break;
        default: PP_NYI; break;
        }

        switch(settings.noteReactivationPolicy){
        case NoteReactivationPolicyDoNotReactivate: reactivate0.setToggleState(true, shutup); break;
        case NoteReactivationPolicyLast: reactivate1.setToggleState(true, shutup); break;
        case NoteReactivationPolicyHighest: reactivate2.setToggleState(true, shutup); break;
        case NoteReactivationPolicyLowest: reactivate3.setToggleState(true, shutup); break;
        case NoteReactivationPolicyFirst: reactivate4.setToggleState(true, shutup); break;
        default: PP_NYI; break;
        }

        switch(settings.noteActivationPolicy){
        case NoteActivationPolicyOnlySilent: activate0.setToggleState(true, shutup); break;
        case NoteActivationPolicyOldest: activate1.setToggleState(true, shutup); break;
        default: PP_NYI; break;
        }

    }

public:
    virtual ~SettingsEditor(){
        subSettings.unsubscribe(subSettingsHandle);
    }

    SettingsEditor(SubValue<PresetSettings>& subSettings_)
        : subSettings(subSettings_)
    {
        addAndMakeVisible(&grid, false);
        subSettingsHandle = subSettings.subscribe([this](const PhasePhckr::PresetSettings& s){
            settings = s;
            applySettings();
        });

        // polyphony
        grid.addComponent(&polyGrid);

        polyphony1.addListener(this);
        polyphony2.addListener(this);
        polyphony4.addListener(this);
        polyphony8.addListener(this);
        polyphony16.addListener(this);
        polyphony32.addListener(this);
        polyphony64.addListener(this);

        polyGrid.addComponent(&polyphony1);
        polyGrid.addComponent(&polyphony2);
        polyGrid.addComponent(&polyphony4);
        polyGrid.addComponent(&polyphony8);
        polyGrid.addComponent(&polyphony16);
        polyGrid.addComponent(&polyphony32);
        polyGrid.addComponent(&polyphony64);

        polyphony1.setButtonText("1");
        polyphony2.setButtonText("2");
        polyphony4.setButtonText("4");
        polyphony8.setButtonText("8");
        polyphony16.setButtonText("16");
        polyphony32.setButtonText("32");
        polyphony64.setButtonText("64");

        polyphony1.setRadioGroupId(1);
        polyphony2.setRadioGroupId(1);
        polyphony4.setRadioGroupId(1);
        polyphony8.setRadioGroupId(1);
        polyphony16.setRadioGroupId(1);
        polyphony32.setRadioGroupId(1);
        polyphony64.setRadioGroupId(1);

        polyphony1.setClickingTogglesState(true);
        polyphony2.setClickingTogglesState(true);
        polyphony4.setClickingTogglesState(true);
        polyphony8.setClickingTogglesState(true);
        polyphony16.setClickingTogglesState(true);
        polyphony32.setClickingTogglesState(true);
        polyphony64.setClickingTogglesState(true);

        // steal policy
        grid.addComponent(&stealGrid);

        steal0.addListener(this);
        steal1.addListener(this);
        steal2.addListener(this);
        steal3.addListener(this);
        steal4.addListener(this);
        steal5.addListener(this);

        stealGrid.addComponent(&steal0);
        stealGrid.addComponent(&steal1);
        stealGrid.addComponent(&steal2);
        stealGrid.addComponent(&steal3);
        stealGrid.addComponent(&steal4);
        stealGrid.addComponent(&steal5);

        steal0.setButtonText("do not steal");
        steal1.setButtonText("oldest");
        steal2.setButtonText("lowest rms");
        steal3.setButtonText("if lower");
        steal4.setButtonText("if higher");
        steal5.setButtonText("youngest");

        steal0.setRadioGroupId(2);
        steal1.setRadioGroupId(2);
        steal2.setRadioGroupId(2);
        steal3.setRadioGroupId(2);
        steal4.setRadioGroupId(2);
        steal5.setRadioGroupId(2);

        steal0.setClickingTogglesState(true);
        steal1.setClickingTogglesState(true);
        steal2.setClickingTogglesState(true);
        steal3.setClickingTogglesState(true);
        steal4.setClickingTogglesState(true);
        steal5.setClickingTogglesState(true);

        // legato mode
        grid.addComponent(&legatoGrid);

        legato0.addListener(this);
        legato1.addListener(this);
        legato2.addListener(this);
        legato3.addListener(this);

        legatoGrid.addComponent(&legato0);
        legatoGrid.addComponent(&legato1);
        legatoGrid.addComponent(&legato2);
        legatoGrid.addComponent(&legato3);

        legato0.setButtonText("retrigger");
        legato1.setButtonText("legato, update velocity");
        legato2.setButtonText("legato, freeze velocity");
        legato3.setButtonText("legato, release velocity");

        legato0.setRadioGroupId(4);
        legato1.setRadioGroupId(4);
        legato2.setRadioGroupId(4);
        legato3.setRadioGroupId(4);

        legato0.setClickingTogglesState(true);
        legato1.setClickingTogglesState(true);
        legato2.setClickingTogglesState(true);
        legato3.setClickingTogglesState(true);

        // reactivation policy
        grid.addComponent(&reactivateGrid);

        reactivate0.addListener(this);
        reactivate1.addListener(this);
        reactivate2.addListener(this);
        reactivate3.addListener(this);
        reactivate4.addListener(this);

        reactivateGrid.addComponent(&reactivate0);
        reactivateGrid.addComponent(&reactivate1);
        reactivateGrid.addComponent(&reactivate2);
        reactivateGrid.addComponent(&reactivate3);
        reactivateGrid.addComponent(&reactivate4);

        reactivate0.setButtonText("do not reactivate");
        reactivate1.setButtonText("reactivate last");
        reactivate2.setButtonText("reactivate highest");
        reactivate3.setButtonText("reactivate lowest");
        reactivate4.setButtonText("reactivate first");

        reactivate0.setRadioGroupId(3);
        reactivate1.setRadioGroupId(3);
        reactivate2.setRadioGroupId(3);
        reactivate3.setRadioGroupId(3);
        reactivate4.setRadioGroupId(3);

        reactivate0.setClickingTogglesState(true);
        reactivate1.setClickingTogglesState(true);
        reactivate2.setClickingTogglesState(true);
        reactivate3.setClickingTogglesState(true);
        reactivate4.setClickingTogglesState(true);

        // activation policy
        grid.addComponent(&activateGrid);

        activate0.addListener(this);
        activate1.addListener(this);

        activateGrid.addComponent(&activate0);
        activateGrid.addComponent(&activate1);

        activate0.setButtonText("only inactive silent");
        activate1.setButtonText("prefer inactive silent, then pick oldest inactive non-silent");

        activate0.setRadioGroupId(5);
        activate1.setRadioGroupId(5);

        activate0.setClickingTogglesState(true);
        activate1.setClickingTogglesState(true);

        applySettings();

        resized();
    }

    void resized() override {
        grid.setBoundsRelative(0.f, 0.f, 1.f, 1.f);
    }

    virtual void buttonClicked (Button* b) override {
        if(b->getToggleState() == false) return;
        else if (b == &polyphony1) settings.polyphony = 1;
        else if (b == &polyphony2) settings.polyphony = 2;
        else if (b == &polyphony4) settings.polyphony = 4;
        else if (b == &polyphony8) settings.polyphony = 8;
        else if (b == &polyphony16) settings.polyphony = 16;
        else if (b == &polyphony32) settings.polyphony = 32;
        else if (b == &polyphony64) settings.polyphony = 64;

        else if (b == &steal0) settings.noteStealPolicy = NoteStealPolicyDoNotSteal;
        else if (b == &steal1) settings.noteStealPolicy = NoteStealPolicyStealOldest;
        else if (b == &steal2) settings.noteStealPolicy = NoteStealPolicyStealLowestRMS;
        else if (b == &steal3) settings.noteStealPolicy = NoteStealPolicyStealIfLower;
        else if (b == &steal4) settings.noteStealPolicy = NoteStealPolicyStealIfHigher;
        else if (b == &steal5) settings.noteStealPolicy = NoteStealPolicyStealYoungest;

        else if (b == &legato0) settings.legatoMode = LegatoModeRetrigger;
        else if (b == &legato1) settings.legatoMode = LegatoModeUpdateVelocity;
        else if (b == &legato2) settings.legatoMode = LegatoModeFreezeVelocity;
        else if (b == &legato3) settings.legatoMode = LegatoModeReleaseVelocity;

        else if (b == &reactivate0) settings.noteReactivationPolicy = NoteReactivationPolicyDoNotReactivate;
        else if (b == &reactivate1) settings.noteReactivationPolicy = NoteReactivationPolicyLast;
        else if (b == &reactivate2) settings.noteReactivationPolicy = NoteReactivationPolicyHighest;
        else if (b == &reactivate3) settings.noteReactivationPolicy = NoteReactivationPolicyLowest;
        else if (b == &reactivate4) settings.noteReactivationPolicy = NoteReactivationPolicyFirst;

        else if (b == &activate0) settings.noteActivationPolicy = NoteActivationPolicyOnlySilent;
        else if (b == &activate1) settings.noteActivationPolicy = NoteActivationPolicyOldest;

        else PP_NYI;

        subSettings.set(subSettingsHandle, settings);
    }

};
