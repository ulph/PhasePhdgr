#pragma once

#include <phasephckr.hpp>
#include "JuceHeader.h"

#include "DirectoryWatcher.hpp"
#include "FileIO.hpp"
#include "Utils.hpp"

#include "ParameterKnob.hpp"
#include "PPGrid.h"

#include <functional>
#include <memory>

using namespace PhasePhckrFileStuff;
using namespace PhasePhckr;
using namespace std;

void storeState(const PresetDescriptor& preset, MemoryBlock& destData, const nlohmann::json& extra);

void loadState(const void* data, int sizeInBytes, PresetDescriptor& preset, nlohmann::json& extra);

void handlePlayHead(Base* effect, AudioPlayHead* playHead, const int blockSize, const float sampleRate, float& barPosition);

struct PPMidiMessage {
    enum class Type {
        Uknown,
        On,
        Off,
        X,
        Y,
        Z,
        NoteZ,
        Expression,
        Breath,
        ModWheel,
        Sustain
    };

    Type type = Type::Uknown;
    int ts = 0;
    int channel = 0;
    int note = 0;
    float value = 0.0f;

    PPMidiMessage(Type type_, int ts_, int channel_, int note_, float value_)
        : type(type_)
        , ts(ts_)
        , channel(channel_)
        , note(note_)
        , value(value_)
    {}

    PPMidiMessage(Type type_, int ts_, int channel_, float value_)
        : type(type_)
        , ts(ts_)
        , channel(channel_)
        , value(value_)
    {}

    PPMidiMessage(Type type_, int ts_, float value_)
        : type(type_)
        , ts(ts_)
        , value(value_)
    {}
};

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
private:
    void processAndRouteMidi(vector<PPMidiMessage>& midiMessageQueue, int blockSize, Synth* synth);
public:
    GeneratingBufferingProcessor()
        : BufferingProcessor(Strategy::AHEAD)
    {}
    virtual void process(AudioSampleBuffer& buffer, vector<PPMidiMessage>& midiMessageQueue, float sampleRate, Synth* synth, Effect* effect, AudioPlayHead* playHead);
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
    LambdaTimer updateTimer;
    vector<ParameterKnob *> parameterKnobs;
public:
    ParameterEditor();
    virtual ~ParameterEditor();
    void addKnob(ParameterKnob* knob);
    void resized() override;
    void stopTimer();
    void startTimer();
};


class PPLookAndFeel : public LookAndFeel_V4 {
private:
    const int trackHalfHeight = 5;
public:
    PPLookAndFeel();

    virtual void drawLinearSlider(
        Graphics& g, 
        int x, 
        int y, 
        int width, 
        int height,
        float sliderPos, 
        float minSliderPos, 
        float maxSliderPos,
        const Slider::SliderStyle style, 
        Slider& slider
    ) override;

};


class PPTabbedComponent : public TabbedComponent {
private:
    function<void(int i, const string & n)> cb;
public:
    PPTabbedComponent(
        const function<void(int i, const string & n)>& cb_
    ) 
        : TabbedComponent(TabbedButtonBar::TabsAtTop)
        , cb(cb_)
    {
        
    }

    virtual void currentTabChanged(
        int newCurrentTabIndex,
        const String & newCurrentTabName
    ) override 
    {
        cb(newCurrentTabIndex, newCurrentTabName.toStdString());
    }

};