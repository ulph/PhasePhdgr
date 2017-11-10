#include "FileEditorBundle.hpp"
#include "FileIO.hpp"
#include "PhasePhckrPluginEditor.h"
#include "Style.hpp"

using namespace PhasePhckrFileStuff;

File FileEditorBundle::makeFullFileFromFilenameLabel() {
    return watcher.getDirectory().getFullPathName() + File::getSeparatorString() + File(filenameLabel.getText() + string(".json")).getFileName();
}

bool FileEditorBundle::isValidFilename() {
    String filename = filenameLabel.getText();
    if (filename.length() == 0) return false;
    if (filename.containsAnyOf(bannedNameCharacters)) return false;
    return true;
}

FileEditorBundle::FileEditorBundle(const string& name, const File& directory, TimeSliceThread& watchThread, ProvideJsonCallBack fileLoadedCallback, GetJsonCallBack fetchJsonCallback)
    : watcher(PhasePhckrFileStuff::getFilter(), watchThread)
    , list(watcher)
    , titleLabel(String(), name)
    , fileLoadedCallback(fileLoadedCallback)
    , fetchJsonCallback(fetchJsonCallback)
{
    list.addListener(this);
    saveButton.addListener(this);
    goToRootButton.addListener(this);
    goOneUpButton.addListener(this);

    fileRoot = directory;

    watcher.setDirectory(directory, true, true);

    addAndMakeVisible(titleLabel);
    addAndMakeVisible(goToRootButton);
    addAndMakeVisible(goOneUpButton);
    addAndMakeVisible(filenameLabel);
    addAndMakeVisible(saveButton);

    addAndMakeVisible(list);

    _stylize(&titleLabel); titleLabel.setJustificationType(Justification::left);
    _stylize(&list);

    _stylize(&filenameLabel);

    goToRootButton.setButtonText("/");
    goOneUpButton.setButtonText("..");
    filenameLabel.setEditable(true);
    saveButton.setButtonText("save");

    resized();
}

void FileEditorBundle::buttonClicked(Button * btn)  {
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

void FileEditorBundle::selectionChanged()  {}

void FileEditorBundle::fileClicked(const File &file, const MouseEvent &e)  {
    // switch directory
    if (file.exists() && !file.existsAsFile()) {
        watcher.setDirectory(file, true, true);
    }
}

void FileEditorBundle::fileDoubleClicked(const File &file)  {
    if (!file.existsAsFile()) return; // nonexistant or a directory
    json j = loadJson(file);
    fileLoadedCallback(j);
    filenameLabel.setText(file.getFileNameWithoutExtension(), sendNotificationAsync);
}

void FileEditorBundle::browserRootChanged(const File &newRoot)  {}

void FileEditorBundle::invalidateSelection() {
    filenameLabel.setText("", sendNotificationAsync);
}

void FileEditorBundle::resized() {
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


FileBrowserPanel::FileBrowserPanel(PhasePhckrAudioProcessor& p)
    : fileWatchThread("editorFileWatchThread")
    , processor(p)
    , subEffectHandle(
        processor.subEffectChain.subscribe(
            [this](const auto& pd) {
                effectFiles.invalidateSelection();
            }
        )
    )
    , subVoiceHandle(
        processor.subVoiceChain.subscribe(
            [this](const auto& pd) {
                voiceFiles.invalidateSelection();
            }
        )
    )
    , voiceFiles(
        "voice files",
        PhasePhckrFileStuff::voicesDir,
        fileWatchThread,
        [this](const json& j) {
            processor.setPatch(VOICE, j);
        },
        [this](void) -> json {
            return processor.getPatch(VOICE);
        }
    )
    , effectFiles(
        "effect files",
        PhasePhckrFileStuff::effectsDir,
        fileWatchThread,
        [this](const json& j) {
            processor.setPatch(EFFECT, j);
        },
        [this](void) -> json {
            return processor.getPatch(EFFECT);
        }
    )
    , presetFiles(
        "preset files",
        PhasePhckrFileStuff::presetsDir,
        fileWatchThread,
        [this](const json& j){
            processor.setPreset(j);
        },
        [this](void) -> json {
            return processor.getPreset();
        }
    )
    , componentFiles(
        "component files",
        PhasePhckrFileStuff::componentsDir,
        fileWatchThread,
        [this](const json& j) {}, // non-sensical
        [this](void) -> json { return json(); } // TODO, load one of the components, somehow
    )
{
    fileWatchThread.startThread();
    fileWatchThread.notify();

    addAndMakeVisible(filesGrid);

    filesGrid.addComponent(&voiceFiles);
    filesGrid.addComponent(&effectFiles);
    filesGrid.addComponent(&presetFiles);

    componentFilesGrid.addComponent(&componentFiles);
    filesGrid.addComponent(&componentFilesGrid);

    resized();
}

void FileBrowserPanel::resized()
{
    filesGrid.setBoundsRelative(0, 0, 1.0f, 1.0f);
    repaint();
}

FileBrowserPanel::~FileBrowserPanel(){
    processor.subVoiceChain.unsubscribe(subVoiceHandle);
    processor.subEffectChain.unsubscribe(subEffectHandle);
}
