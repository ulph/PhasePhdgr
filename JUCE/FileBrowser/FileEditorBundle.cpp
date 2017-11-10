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
