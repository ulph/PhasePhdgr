#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include <vector>

#include <phasephckr.hpp>

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

class PhasePhckrEditorBase  : public AudioProcessorEditor, public DragAndDropContainer
{
public:
    PhasePhckrEditorBase ( PhasePhckrProcessorBase& );
    virtual ~PhasePhckrEditorBase();

    void paint (Graphics&) override;
    void resized() override;

    friend Parameters;

protected:
    std::vector<ScopeI *> scopes; // TODO, memory management from hell
    PPGrid scopePPGrid;
    PPTabbedComponent mainFrame;

    PhasePhckrProcessorBase& processor;

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhasePhckrEditorBase)
};

class PhasePhckrEditor: public PhasePhckrEditorBase {
public:
    PhasePhckrEditor ( PhasePhckrProcessorBase& p);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhasePhckrEditor)
private:
    PPGGrid voiceScopeGrid;
    PPGGrid synthScopeOutGrid;
    PPGGrid effectScopeOutGrid;
};

class PhasePhckrEditorFX: public PhasePhckrEditorBase {
public:
    PhasePhckrEditorFX ( PhasePhckrProcessorBase& p);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhasePhckrEditorFX)
private:
    PPGGrid effectScopeInGrid;
    PPGGrid effectScopeOutGrid;
};

#endif  // PLUGINEDITOR_H_INCLUDED
