#include "FileEditorBundle.hpp"
#include "FileIO.hpp"
#include "PhasePhckrPluginEditor.h"

using namespace PhasePhckrFileStuff;

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
    filesGrid.addComponent(&componentFiles);
}

FileBrowserPanel::~FileBrowserPanel(){
    processor.subVoiceChain.unsubscribe(subVoiceHandle);
    processor.subEffectChain.unsubscribe(subEffectHandle);
}
