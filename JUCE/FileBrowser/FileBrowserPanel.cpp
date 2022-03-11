#include "FileBrowserPanel.hpp"

FileBrowserPanel::FileBrowserPanel(PhasePhckrProcessorBase& p)
    : fileWatchThread("editorFileWatchThread")
    , processor(p)
    , subEffectHandle(
        processor.getPropagator(SynthGraphType::EFFECT).subscribe(
            [this](const auto& pd) {
                effectFiles.invalidateSelection();
                updateComponentMap(effectComponents, effectDocView, pd);
            }
        )
    )
    , subVoiceHandle(
        processor.getPropagator(SynthGraphType::VOICE).subscribe(
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
            processor.setPatch(SynthGraphType::VOICE, j);
        },
        [this](void) -> json {
            return processor.getPatch(SynthGraphType::VOICE, true);
        }
    )
    , effectFiles(
        "effect files",
        PhasePhckrFileStuff::effectsDir,
        fileWatchThread,
        [this](const string& n, const json& j) {
            processor.setPatch(SynthGraphType::EFFECT, j);
        },
        [this](void) -> json {
            return processor.getPatch(SynthGraphType::EFFECT, true);;
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
    fileWatchThread.startThread();
    fileWatchThread.notify();

    addAndMakeVisible(filesPPGrid);

    if(processor.isSynth()){
        filesPPGrid.addComponent(&voiceFiles);
        filesPPGrid.addComponent(&presetFiles);
    }

    filesPPGrid.addComponent(&effectFiles);

    filesPPGrid.addComponent(&componentFilesPPGrid);
    componentFilesPPGrid.addComponent(&componentFiles);
    componentFilesPPGrid.addComponent(&docViewTab);

    if(processor.isSynth()){
        docViewTab.addTab("voice", Colours::black, &voiceDocView, false);
    }

    docViewTab.addTab("effect", Colours::black, &effectDocView, false);

    resized();
}

void FileBrowserPanel::resized()
{
    filesPPGrid.setBoundsRelative(0, 0, 1.0f, 1.0f);
}

FileBrowserPanel::~FileBrowserPanel(){
    if(processor.isSynth()){
        processor.getPropagator(SynthGraphType::VOICE).unsubscribe(subVoiceHandle);
    }
    processor.getPropagator(SynthGraphType::EFFECT).unsubscribe(subEffectHandle);
}