#pragma once

#include "PhasePhckrPluginProcessor.h"

#include "FileIO.hpp"
#include "DirectoryWatcher.hpp"
#include "PhasePhckrGrid.h"

#include "PatchEditor.hpp" // TODO for _stylize

class PhasePhckrEditor;

typedef std::function<void(const string& name, const nlohmann::json&)> ProvideJsonCallBack;
typedef std::function<nlohmann::json(void)> GetJsonCallBack;

using namespace PhasePhckrFileStuff;

class FileEditorBundle : public Component, public ButtonListener, public FileBrowserListener
{

    // TODO, thread safety between checks and usages of checks ... maybe

private:

    Label titleLabel;
    TextButton goToRootButton;
    TextButton goOneUpButton;
    Label filenameLabel;
    TextButton saveButton;

    DirectoryContentsList watcher;
    FileListComponent list;

    ProvideJsonCallBack fileLoadedCallback;
    GetJsonCallBack fetchJsonCallback;

    File fileRoot;

    File makeFullFileFromFilenameLabel();
    bool isValidFilename();

public:
    virtual void buttonClicked(Button * btn) override;
    virtual void selectionChanged() override;
    virtual void fileClicked(const File &file, const MouseEvent &e) override;
    virtual void fileDoubleClicked(const File &file) override;
    virtual void browserRootChanged(const File &newRoot) override;
    void invalidateSelection();
    void setFileName(const string& newName);

    FileEditorBundle(const string& name, const File& directory, TimeSliceThread& watchThread, ProvideJsonCallBack fileLoadedCallback, GetJsonCallBack fetchJsonCallback);

    virtual void resized() override;

};


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

    void updateComponentMap(map<string, ComponentDescriptor>& c, DocView& d, const PatchDescriptor& p);
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
