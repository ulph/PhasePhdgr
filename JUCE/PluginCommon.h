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
    {}
    virtual void process(AudioSampleBuffer& buffer, float sampleRate, Effect* synth, AudioPlayHead* playHead);
};


class InputBufferingProcessor: public BufferingProcessor{
public:
    InputBufferingProcessor() 
        : BufferingProcessor(Strategy::BUFFERING)
    {}
    virtual void process(AudioSampleBuffer& buffer, float sampleRate, Effect* effect, AudioPlayHead* playHead);
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
