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
