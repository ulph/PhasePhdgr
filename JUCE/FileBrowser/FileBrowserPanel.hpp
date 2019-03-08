#pragma once

#include "FileEditorBundle.hpp"

class FileBrowserPanel : public Component {
private:
    PhasePhckrProcessorBase & processor;
    int subVoiceHandle;
    int subEffectHandle;
    TimeSliceThread fileWatchThread;
    PPGrid filesPPGrid;
    FileEditorBundle voiceFiles;
    FileEditorBundle effectFiles;
    FileEditorBundle presetFiles;

    PPGrid componentFilesPPGrid;
    FileEditorBundle componentFiles;
    TabbedComponent docViewTab;
    map<string, ComponentDescriptor> voiceComponents;
    map<string, ComponentDescriptor> effectComponents;
    DocView voiceDocView;
    DocView effectDocView;
    ComponentDescriptor selectedComponent;

public:
    FileBrowserPanel(PhasePhckrProcessorBase& editor);
    virtual ~FileBrowserPanel();
    void resized() override;
};
