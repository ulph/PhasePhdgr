#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include <vector>

#include <phasephdgr.hpp>

#include "PluginProcessor.h"
#include "ScopeView.h"
#include "DirectoryWatcher.hpp"
#include "FileBrowserPanel.hpp"
#include "GraphEditor.hpp"

#include "Utils.hpp"
#include "PatchEditor.hpp"

#include "ParameterKnob.hpp"

#include "SettingsEditor.hpp"

#include <juce_audio_processors/juce_audio_processors.h>

using namespace juce;

class PhasePhdgrEditorBase  : public AudioProcessorEditor, public DragAndDropContainer
{
public:
    PhasePhdgrEditorBase ( PhasePhdgrProcessorBase& );
    virtual ~PhasePhdgrEditorBase();

    void paint (Graphics&) override;
    void resized() override;

    friend Parameters;

protected:
    std::vector<ScopeI *> scopes; // TODO, memory management from hell
    PPGrid scopePPGrid;
    PPTabbedComponent mainFrame;

    PhasePhdgrProcessorBase& processor;

    ParameterEditor parameterEditor;

    FileBrowserPanel fileBrowserPanel;

    PatchEditor voiceEditor;
    PatchEditor effectEditor;

    SettingsEditor settingsEditor;

private:

    bool inited = false;

#if INTERCEPT_STD_STREAMS
    InterceptStringStream coutIntercept;
    InterceptStringStream cerrIntercept;
    TextEditor coutView;
    TextEditor cerrView;
    PPGrid debugTab;
    LambdaTimer* debugViewUpdateTimer;
#endif

    LambdaTimer guiUpdateTimer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhasePhdgrEditorBase)
};

class PhasePhdgrEditor: public PhasePhdgrEditorBase {
public:
    PhasePhdgrEditor ( PhasePhdgrProcessorBase& p);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhasePhdgrEditor)
private:
    PPGGrid voiceScopeGrid;
    PPGGrid synthScopeOutGrid;
    PPGGrid effectScopeOutGrid;
};

class PhasePhdgrEditorFX: public PhasePhdgrEditorBase {
public:
    PhasePhdgrEditorFX ( PhasePhdgrProcessorBase& p);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhasePhdgrEditorFX)
private:
    PPGGrid effectScopeInGrid;
    PPGGrid effectScopeOutGrid;
};

#endif  // PLUGINEDITOR_H_INCLUDED
