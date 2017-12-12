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
                subComponentRegister.set(-1, componentRegister);
            }
            catch (const std::exception& e) {
                (void)e;
                continue;
                assert(0);
            }
        }
    }

};

class ParameterPages : public TabbedComponent {
public:
    virtual void currentTabChanged(int newCurrentTabIndex, const String &newCurrentTabName) override;
    ParameterPages() : TabbedComponent(TabbedButtonBar::TabsAtBottom) {}
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