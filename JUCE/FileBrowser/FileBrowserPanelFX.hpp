#pragma once 

#include "FileEditorBundle.hpp"

class FileBrowserPanelFX : public Component {
private:
    PhasePhckrProcessorFX& processor;
    int subEffectHandle;
    TimeSliceThread fileWatchThread;
    PPGrid filesPPGrid;
    FileEditorBundle effectFiles;

    PPGrid componentFilesPPGrid;
    FileEditorBundle componentFiles;
    TabbedComponent docViewTab;
    map<string, ComponentDescriptor> voiceComponents;
    map<string, ComponentDescriptor> effectComponents;
    DocView effectDocView;
    ComponentDescriptor selectedComponent;

public:
    FileBrowserPanelFX(PhasePhckrProcessorFX& editor);
    virtual ~FileBrowserPanelFX();
    void resized() override;
};