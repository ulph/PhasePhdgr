#include "FileBrowserPanelFX.hpp"

FileBrowserPanelFX::FileBrowserPanelFX(PhasePhckrProcessorFX& p)
    : fileWatchThread("editorFxFileWatchThread")
    , processor(p)
    , subEffectHandle(
        processor.subEffectChain.subscribe(
            [this](const auto& pd) {
                effectFiles.invalidateSelection();
                updateComponentMap(effectComponents, effectDocView, pd);
            }
        )
    )
    , effectFiles(
        "effect files",
        PhasePhckrFileStuff::effectsDir,
        fileWatchThread,
        [this](const string& n, const json& j) {
            processor.setPatch(j);
        },
        [this](void) -> json {
            return processor.getPatch();
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

    filesPPGrid.addComponent(&effectFiles);

    filesPPGrid.addComponent(&componentFilesPPGrid);
    componentFilesPPGrid.addComponent(&componentFiles);
    componentFilesPPGrid.addComponent(&docViewTab);
    docViewTab.addTab("effect", Colours::black, &effectDocView, false);

    resized();
}

void FileBrowserPanelFX::resized()
{
    filesPPGrid.setBoundsRelative(0, 0, 1.0f, 1.0f);
}

FileBrowserPanelFX::~FileBrowserPanelFX(){
    processor.subEffectChain.unsubscribe(subEffectHandle);
}
