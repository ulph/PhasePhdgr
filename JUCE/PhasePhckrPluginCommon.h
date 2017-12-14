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
    float barPosition = 0.f;
    int lastBlockSize = 0;

    int carryOverOutputSamples = 0;
    float* carryOverOutputBlockBuffer[2] = { nullptr };

    void process(AudioSampleBuffer& buffer, float sampleRate, Effect* synth, AudioPlayHead* playHead) {
        if (buffer.getNumSamples() != lastBlockSize) {
            // if blocksize changes, align
            carryOverOutputSamples = 0;
        }
        lastBlockSize = buffer.getNumSamples();

        assert(carryOverOutputSamples >= 0);
        assert(carryOverOutputSamples <= Synth::internalBlockSize());

        int bufferOffset = (Synth::internalBlockSize() - carryOverOutputSamples) % Synth::internalBlockSize();

        const int blockSize = buffer.getNumSamples() - bufferOffset;
        const int alignedBlockSize = Synth::internalBlockSize() * (blockSize / Synth::internalBlockSize());

        assert(alignedBlockSize >= 0);
        assert(alignedBlockSize <= blockSize);

        int last_carryOverOutputSamples = carryOverOutputSamples;
        carryOverOutputSamples = blockSize - alignedBlockSize;

        assert((bufferOffset + alignedBlockSize + carryOverOutputSamples) == buffer.getNumSamples());

        // samples from last call
        if (bufferOffset > 0) {
            auto* l = buffer.getWritePointer(0);
            auto* r = buffer.getWritePointer(1);
            for (int i = 0; i < bufferOffset; ++i) {
                l[i] = carryOverOutputBlockBuffer[0][last_carryOverOutputSamples + i];
                r[i] = carryOverOutputBlockBuffer[1][last_carryOverOutputSamples + i];
                carryOverOutputBlockBuffer[0][last_carryOverOutputSamples + i] = 0.f;
                carryOverOutputBlockBuffer[1][last_carryOverOutputSamples + i] = 0.f;
            }
        }

        // samples, if any, that fits a multiple of Synth::internalBlockSize
        if (alignedBlockSize > 0) {
            handlePlayHead(synth, playHead, alignedBlockSize, sampleRate, barPosition);
            synth->update(buffer.getWritePointer(0, bufferOffset), buffer.getWritePointer(1, bufferOffset), alignedBlockSize, sampleRate);
        }

        // if not all samples fit, calculate a new frame and store
        if (carryOverOutputSamples > 0) {
            handlePlayHead(synth, playHead, Synth::internalBlockSize(), sampleRate, barPosition);
            synth->update(carryOverOutputBlockBuffer[0], carryOverOutputBlockBuffer[1], Synth::internalBlockSize(), sampleRate);
            auto* l = buffer.getWritePointer(0, bufferOffset + alignedBlockSize);
            auto* r = buffer.getWritePointer(1, bufferOffset + alignedBlockSize);
            for (int i = 0; i < carryOverOutputSamples; ++i) {
                l[i] = carryOverOutputBlockBuffer[0][i];
                r[i] = carryOverOutputBlockBuffer[1][i];
                carryOverOutputBlockBuffer[0][i] = 0.f;
                carryOverOutputBlockBuffer[1][i] = 0.f;
            }
        }
    }

    BufferingProcessor() {
        carryOverOutputBlockBuffer[0] = new float[Synth::internalBlockSize()]{ 0.0f };
        carryOverOutputBlockBuffer[1] = new float[Synth::internalBlockSize()]{ 0.0f };
    }

    ~BufferingProcessor() {
        delete carryOverOutputBlockBuffer[0];
        delete carryOverOutputBlockBuffer[1];
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