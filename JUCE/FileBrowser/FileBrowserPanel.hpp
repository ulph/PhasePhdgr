#pragma once

#include "FileEditorBundle.hpp"

class FileBrowserPanel : public Component {
private:
    PhasePhckrProcessor& processor;
    int subVoiceHandle;
    int subEffectHandle;
    TimeSliceThread fileWatchThread;
    PhasePhckrGrid filesGrid;
    FileEditorBundle voiceFiles;
    FileEditorBundle effectFiles;
    FileEditorBundle presetFiles;

    PhasePhckrGrid componentFilesGrid;
    FileEditorBundle componentFiles;
    TabbedComponent docViewTab;
    map<string, ComponentDescriptor> voiceComponents;
    map<string, ComponentDescriptor> effectComponents;
    DocView voiceDocView;
    DocView effectDocView;
    ComponentDescriptor selectedComponent;

public:
    FileBrowserPanel(PhasePhckrProcessor& editor);
    virtual ~FileBrowserPanel();
    void resized() override;
};
