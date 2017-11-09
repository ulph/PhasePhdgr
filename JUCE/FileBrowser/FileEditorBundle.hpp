#pragma once

#include "FileIO.hpp"
#include "DirectoryWatcher.hpp"

class FileEditorBundle : public Component, public ButtonListener, public FileBrowserListener
{

private:
    Label titleLabel;
    TextButton saveButton;
    DirectoryContentsList watcher;
    FileListComponent list;
    Label filenameLabel;
//    json currentLoadedJson;

    PhasePhckrFileStuff::JsonCallBack fileLoadedCallback;

public:
    virtual void buttonClicked(Button * btn) override {
    }

    virtual void selectionChanged() {}
    virtual void fileClicked(const File &file, const MouseEvent &e) {
        // do nought
    }
    virtual void fileDoubleClicked(const File &file) {
        json j = loadJson(file);
        fileLoadedCallback(j);
        filenameLabel.setText(file.getFileNameWithoutExtension(), sendNotificationAsync);
    }
    virtual void browserRootChanged(const File &newRoot) {}

    /*
    virtual void setJson(const json & json) {
        currentLoadedJson = json;
    }
    */

    FileEditorBundle(const string& name, const File& directory, TimeSliceThread& watchThread, PhasePhckrFileStuff::JsonCallBack fileLoadedCallback)
        : watcher(PhasePhckrFileStuff::getFilter(), watchThread)
        , list(watcher)
        , titleLabel(String(), name)
        , fileLoadedCallback(fileLoadedCallback)
    {
        list.addListener(this);
        saveButton.addListener(this);

        watcher.setDirectory(directory, true, true);

        addAndMakeVisible(titleLabel);
        addAndMakeVisible(filenameLabel);
        addAndMakeVisible(saveButton);

        addAndMakeVisible(list);

        _stylize(&titleLabel); titleLabel.setJustificationType(Justification::left);
        _stylize(&list);

        _stylize(&filenameLabel);

        filenameLabel.setEditable(true);
        saveButton.setButtonText("save");

        resized();
    }

    virtual void resized() override {
        const int rowHeightPx = 30;
        const int titleWidthPx = 100;
        const int buttonWidthPx = 100;

        auto bounds = getBounds();
        auto boundsHeight = bounds.getHeight();
        auto boundsWidth = bounds.getWidth();

        float rowHeight = 0.01f;
        float titleWidth = 0.01f;
        float buttonWidth = 0.01f;

        if (boundsHeight > 0) {
            rowHeight = (float)rowHeightPx / (float)boundsHeight;
        }

        if (boundsWidth > 0) {
            titleWidth = (float)titleWidthPx / (float)boundsWidth;
            buttonWidth = (float)buttonWidthPx / (float)boundsWidth;
        }

        titleLabel.setBoundsRelative(0.f, 0.f, titleWidth, rowHeight);
        filenameLabel.setBoundsRelative(titleWidth, 0.f, 1.f - titleWidth - buttonWidth, rowHeight);
        saveButton.setBoundsRelative(1.f - buttonWidth, 0.f, buttonWidth, rowHeight);

        list.setBoundsRelative(0.f, rowHeight, 1.f, 1.f - rowHeight);
    }

};