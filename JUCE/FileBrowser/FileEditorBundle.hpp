#pragma once

#include "PluginProcessor.h"

#include "FileIO.hpp"
#include "DirectoryWatcher.hpp"
#include "PPGrid.h"

#include "PatchEditor.hpp"

class PhasePhdgrEditor;

typedef std::function<void(const string& name, const nlohmann::json&)> ProvideJsonCallBack;
typedef std::function<nlohmann::json(void)> GetJsonCallBack;

using namespace PhasePhdgrFileStuff;

class FileEditorBundle : public GroupComponent, public Button::Listener, public FileBrowserListener
{

    // TODO, thread safety between checks and usages of checks ... maybe

private:

    TextButton goToRootButton;
    TextButton goOneUpButton;
    Label filenameLabel;
    TextButton saveButton;

    DirectoryContentsList watcher;
    FileListComponent list;

    ProvideJsonCallBack fileLoadedCallback;
    GetJsonCallBack fetchJsonCallback;

    bool allowsOverwrites = true;
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
    void allowOverwrites(bool allow) { allowsOverwrites = allow; }

    FileEditorBundle(const string& name, const File& directory, TimeSliceThread& watchThread, ProvideJsonCallBack fileLoadedCallback, GetJsonCallBack fetchJsonCallback);

    virtual void resized() override;

};

void updateComponentMap(map<string, ComponentDescriptor>& c, DocView& d, const PatchDescriptor& p);
