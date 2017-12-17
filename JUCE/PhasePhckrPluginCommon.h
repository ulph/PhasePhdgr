#pragma once

#include <phasephckr.hpp>
#include "JuceHeader.h"

#include "DirectoryWatcher.hpp"
#include "FileIO.hpp"
#include "Utils.hpp"

#include "ParameterKnob.hpp"
#include "PhasePhckrGrid.h"

#include <functional>

using namespace PhasePhckrFileStuff;
using namespace PhasePhckr;
using namespace std;

void proccessHandleCommonMidi(Effect* effect);

void handlePlayHead(Effect* effect, AudioPlayHead* playHead, const int blockSize, const float sampleRate, float& barPosition);

struct ProcessorFileThings {
    TimeSliceThread fileWatchThread;
    DirectoryContentsList componentDirectoryWatcher;
    StupidFileChangeListener componentFilesListener;

    PhasePhckr::ComponentRegister componentRegister;
    SubValue<PhasePhckr::ComponentRegister>& subComponentRegister;

    ProcessorFileThings(SubValue<PhasePhckr::ComponentRegister>& subComponentRegister_)
        : subComponentRegister(subComponentRegister_)
        , fileWatchThread("processorFileThingsThread")
        , componentDirectoryWatcher(getFilter(), fileWatchThread)
        , componentFilesListener(StupidFileListCallBack([this](const DirectoryContentsList* d) {updateComponentRegister(d); }))
    {
        fileWatchThread.startThread();
        componentDirectoryWatcher.addChangeListener(&componentFilesListener);
        componentDirectoryWatcher.setDirectory(componentsDir, true, true);
    }

    void updateComponentRegister(const DirectoryContentsList* d)
    {
        for (int i = 0; i<d->getNumFiles(); i++) {
            const File& f = d->getFile(i);
            if (!f.existsAsFile()) continue; // TODO, recurse into subdirs
            String p = f.getRelativePathFrom(componentsDir);
            string n = string(&componentMarker, 1) + p.dropLastCharacters(5).toUpperCase().toStdString(); // remove .json
            string s = f.loadFileAsString().toStdString();
            try {
                json j = json::parse(s.c_str());
                ComponentDescriptor cd = j;
                componentRegister.registerComponent(n, cd);
                if (!d->isStillLoading()) {
                    subComponentRegister.set(-1, componentRegister);
                }
            }
            catch (const std::exception& e) {
                (void)e;
                continue;
                assert(0);
            }
        }
    }

};


struct BufferingProcessor {

    float barPosition = 0;
    int inputBufferSamples = 0;
    int outputBufferSamples = Effect::internalBlockSize();
    float* inputBuffer[2] = { nullptr }; // use vectors instead?
    float* outputBuffer[2] = { nullptr };
    AudioSampleBuffer scratchBuffer;

    void process(AudioSampleBuffer& buffer, float sampleRate, Effect* effect, AudioPlayHead* playHead) {
        /*
        fill up (as many samples we can) from _buffer_ starting at _inputBufferSamples_, and if full
        process _inputBuffer_ in place
        process as many remaining multiple _internalBlockSize_ of _buffer_ in-place as we can
        make sure _scratchBuffer_ is at least _blockSize_ big
        fill up _scratchBuffer_ with the data processed in _buffer_
        copy any remaining unprocessed samples in _buffer_ to _inputBuffer_, set _inputBufferSamples_

        copy _outputBufferSamples_ from _outputBuffer_ to start of _buffer_ as far as possible, set _outputBufferSamples_, _bufferSamples_
        if _bufferSamples_ < _blockSize_
        fill upp _buffer_ with _scratchBuffer_ as far possible
        top up _outputBuffer_ with any unused processed samples in _scratchBuffer_, set _outputBufferSamples_

        assert neither _inputBufferSamples_ nor _outputBufferSamples_ > _internalBlockSize_
        in fact; assert _inputBufferSamples_ + _outputBufferSamples_ <= _internalBlockSize_
        assert _bufferSamples_ == _blockSize_

        ... alterantively, use a smaller scratch buffer (same as internal block size?) do all the buffer shuffling piece wise (should be possible)
        ... or, alternatively still -- use much larger input and output buffers and do not use the scratch buffer (have to be arbitrary size though)
        */

        // pre checks
        assert(buffer.getNumChannels() >= 2);

        if (scratchBuffer.getNumChannels() != buffer.getNumChannels() || scratchBuffer.getNumSamples() != 2 * buffer.getNumSamples()) {
            scratchBuffer.setSize(buffer.getNumChannels(), 2 * buffer.getNumSamples(), true, true, true);
        }

        const int blockSize = buffer.getNumSamples();
        const int internalBlockSize = Effect::internalBlockSize();

        // buffer input and process, bump to scratchBuffer
        int i = 0;
        for (i; i < internalBlockSize - inputBufferSamples && i < blockSize; i++) {
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
            for (o; o < outputBufferSamples && o < blockSize; o++) {
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

    BufferingProcessor() {
        inputBuffer[0] = new float[Synth::internalBlockSize()]{ 0.0f };
        inputBuffer[1] = new float[Synth::internalBlockSize()]{ 0.0f };
        outputBuffer[0] = new float[Synth::internalBlockSize()]{ 0.0f };
        outputBuffer[1] = new float[Synth::internalBlockSize()]{ 0.0f };
    }

    ~BufferingProcessor() {
        delete[] inputBuffer[0];
        delete[] inputBuffer[1];
        delete[] outputBuffer[0];
        delete[] outputBuffer[1];
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

class PhasePhckrParameterEditor : public Component, public DragAndDropContainer {
private:
    ParameterPages pageTabs;
    vector<PhasePhckrGrid*> pages;
    int knobCtr = 0;
    const vector<float> rowLayout = { 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f };
public:
    PhasePhckrParameterEditor();
    virtual ~PhasePhckrParameterEditor();
    void addKnob(ParameterKnob* knob);
    void resized() override;
};