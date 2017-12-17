#include "FileBrowserPanel.hpp"

FileBrowserPanel::FileBrowserPanel(PhasePhckrProcessor& p)
    : fileWatchThread("editorFileWatchThread")
    , processor(p)
    , subEffectHandle(
        processor.subEffectChain.subscribe(
            [this](const auto& pd) {
                effectFiles.invalidateSelection();
                updateComponentMap(effectComponents, effectDocView, pd);
            }
        )
    )
    , subVoiceHandle(
        processor.subVoiceChain.subscribe(
            [this](const auto& pd) {
                voiceFiles.invalidateSelection();
                updateComponentMap(voiceComponents, voiceDocView, pd);
            }
        )
    )
    , voiceFiles(
        "voice files",
        PhasePhckrFileStuff::voicesDir,
        fileWatchThread,
        [this](const string& n, const json& j) {
            processor.setPatch(VOICE, j);
        },
        [this](void) -> json {
            return processor.getPatch(VOICE, true);
        }
    )
    , effectFiles(
        "effect files",
        PhasePhckrFileStuff::effectsDir,
        fileWatchThread,
        [this](const string& n, const json& j) {
            processor.setPatch(EFFECT, j);
        },
        [this](void) -> json {
            return processor.getPatch(EFFECT, true);;
        }
    )
    , presetFiles(
        "preset files",
        PhasePhckrFileStuff::presetsDir,
        fileWatchThread,
        [this](const string& n, const json& j){
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
        [this](const string& n, const json& j) {},
        [this](void) -> json {
            return selectedComponent;
        }
    )
    , docViewTab(TabbedButtonBar::TabsAtTop)
    , voiceDocView(
        [this](const string& name, const MouseEvent& me){
            selectedComponent = voiceComponents[name];
            componentFiles.setFileName(name.substr(1));
        }
    )
    , effectDocView(
        [this](const string& name, const MouseEvent& me){
            selectedComponent = effectComponents[name];
            componentFiles.setFileName(name.substr(1));
        }
    )
{
    componentFiles.allowOverwrites(false);

    fileWatchThread.startThread();
    fileWatchThread.notify();

    addAndMakeVisible(filesGrid);

    filesGrid.addComponent(&voiceFiles);
    filesGrid.addComponent(&effectFiles);
    filesGrid.addComponent(&presetFiles);

    filesGrid.addComponent(&componentFilesGrid);
    componentFilesGrid.addComponent(&componentFiles);
    componentFilesGrid.addComponent(&docViewTab);
    docViewTab.addTab("voice", Colours::black, &voiceDocView, false);
    docViewTab.addTab("effect", Colours::black, &effectDocView, false);

    resized();
}

void FileBrowserPanel::resized()
{
    filesGrid.setBoundsRelative(0, 0, 1.0f, 1.0f);
}

FileBrowserPanel::~FileBrowserPanel(){
    processor.subVoiceChain.unsubscribe(subVoiceHandle);
    processor.subEffectChain.unsubscribe(subEffectHandle);
}