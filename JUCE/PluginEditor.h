#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include <vector>

#include <phasephckr.hpp>
#include "JuceHeader.h"

#include "PluginProcessor.h"
#include "ScopeView.h"
#include "DirectoryWatcher.hpp"
#include "FileBrowserPanel.hpp"
#include "GraphEditor.hpp"

#include "Utils.hpp"
#include "PatchEditor.hpp"

#include "ParameterKnob.hpp"

#include "SettingsEditor.hpp"

class PhasePhckrEditor  : public AudioProcessorEditor, public DragAndDropContainer
{
public:
    PhasePhckrEditor (
        PhasePhckrProcessor&
     );
    ~PhasePhckrEditor();

    void paint (Graphics&) override;
    void resized() override;

    friend Parameters;

private:
    PPLookAndFeel lookAndFeel;

    PhasePhckrProcessor& processor;

    ScopeView voiceScopeL;
    ScopeView voiceScopeR;
    XYScopeView voiceScopeXY;
    ScopeView outputScopeL;
    ScopeView outputScopeR;
    XYScopeView outputScopeXY;

    TabbedComponent mainFrame;
    PPGGrid voiceScopeGrid;
    PPGGrid effectScopeGrid;
    PPGrid scopePPGrid;
    ParameterEditor parameterEditor;

    FileBrowserPanel fileBrowserPanel;

    PatchEditor voiceEditor;
    PatchEditor effectEditor;

#if INTERCEPT_STD_STREAMS
    InterceptStringStream coutIntercept;
    InterceptStringStream cerrIntercept;
    TextEditor coutView;
    TextEditor cerrView;
    PPGrid debugTab;
    LambdaTimer* debugViewUpdateTimer;
#endif

    vector<ParameterKnob *> parameterKnobs;
    LambdaTimer guiUpdateTimer;

    SettingsEditor settingsEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhasePhckrEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED
