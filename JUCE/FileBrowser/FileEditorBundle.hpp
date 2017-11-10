#pragma once

#include "PhasePhckrPluginProcessor.h"

#include "FileIO.hpp"
#include "DirectoryWatcher.hpp"
#include "PhasePhckrGrid.h"

#include "PatchEditor.hpp" // TODO for _stylize

typedef std::function<void(const nlohmann::json&)> ProvideJsonCallBack;
typedef std::function<nlohmann::json(void)> GetJsonCallBack;

const String bannedNameCharacters = " @!?.=-";

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
    virtual void buttonClicked(Button * btn) override {
        if (btn == &saveButton) {
            if (!isValidFilename()) return;

            File targetFile = makeFullFileFromFilenameLabel();
            if (targetFile.exists()) return;

            auto j = fetchJsonCallback();
            storeJson(targetFile, j);
            watcher.refresh();
        }
        else if (btn == &goToRootButton) {
            watcher.setDirectory(fileRoot, true, true);
        }
        else if (btn == &goOneUpButton) {
            auto currDir = watcher.getDirectory();
            if (currDir.getFullPathName() == fileRoot.getFullPathName()) return;
            auto oneUp = currDir.getParentDirectory();
            watcher.setDirectory(oneUp, true, true);
        }
    }
    virtual void selectionChanged() override {}
    virtual void fileClicked(const File &file, const MouseEvent &e) override {
        // switch directory
        if (file.exists() && !file.existsAsFile()) {
            watcher.setDirectory(file, true, true);
        }
    }
    virtual void fileDoubleClicked(const File &file) override {
        if (!file.existsAsFile()) return; // nonexistant or a directory
        json j = loadJson(file);
        fileLoadedCallback(j);
        filenameLabel.setText(file.getFileNameWithoutExtension(), sendNotificationAsync);
    }
    virtual void browserRootChanged(const File &newRoot) override {}
    
    void invalidateSelection() {
        filenameLabel.setText("", sendNotificationAsync);
    }

    FileEditorBundle(const string& name, const File& directory, TimeSliceThread& watchThread, ProvideJsonCallBack fileLoadedCallback, GetJsonCallBack fetchJsonCallback);

    virtual void resized() override {
        const int rowHeightPx = 30;
        const int titleWidthPx = 100;
        const int saveButtonWidthPx = 100;
        const int dirButtonWidthPx = 30;

        auto bounds = getBounds();
        auto boundsHeight = bounds.getHeight();
        auto boundsWidth = bounds.getWidth();

        float rowHeight = 0.01f;
        float titleWidth = 0.01f;
        float saveButtonWidth = 0.01f;
        float dirButtonWidth = 0.01f;

        if (boundsHeight > 0) {
            rowHeight = (float)rowHeightPx / (float)boundsHeight;
        }

        if (boundsWidth > 0) {
            titleWidth = (float)titleWidthPx / (float)boundsWidth;
            saveButtonWidth = (float)saveButtonWidthPx / (float)boundsWidth;
            dirButtonWidth = (float)dirButtonWidthPx / (float)boundsWidth;
        }

        float filenameLabelWidth = 1.f - titleWidth - 2 * dirButtonWidth - saveButtonWidth;

        titleLabel.setBoundsRelative(0.f, 0.f, titleWidth, rowHeight);
        goToRootButton.setBoundsRelative(titleWidth, 0.f, dirButtonWidth, rowHeight);
        goOneUpButton.setBoundsRelative(titleWidth + dirButtonWidth, 0.f, dirButtonWidth, rowHeight);
        filenameLabel.setBoundsRelative(titleWidth + 2*dirButtonWidth, 0.f, filenameLabelWidth, rowHeight);
        saveButton.setBoundsRelative(1.f - saveButtonWidth, 0.f, saveButtonWidth, rowHeight);

        list.setBoundsRelative(0.f, rowHeight, 1.f, 1.f - rowHeight);
    }

};

class PhasePhckrAudioProcessorEditor;

class FileBrowserPanel : public Component {
private:
    PhasePhckrAudioProcessor& processor;
    int subVoiceHandle;
    int subEffectHandle;
    TimeSliceThread fileWatchThread;
    PhasePhckrGrid filesGrid;
    FileEditorBundle voiceFiles;
    FileEditorBundle effectFiles;
    FileEditorBundle presetFiles;

    PhasePhckrGrid componentFilesGrid;
    FileEditorBundle componentFiles;

public:
    FileBrowserPanel(PhasePhckrAudioProcessor& editor);
    virtual ~FileBrowserPanel();
    void resized() override;
};
