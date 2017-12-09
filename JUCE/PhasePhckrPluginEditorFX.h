#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include <vector>

#include <phasephckr.hpp>
#include "JuceHeader.h"

#include "PhasePhckrPluginProcessorFX.h"
#include "PhasePhckrScope.h"
#include "DirectoryWatcher.hpp"
// #include "FileEditorBundle.hpp"
#include "GraphEditor.hpp"

#include "Utils.hpp"
#include "PatchEditor.hpp"

#include "ParameterKnob.hpp"

class PhasePhckrAudioProcessorEditorFX  : public AudioProcessorEditor, public DragAndDropContainer
{
public:
    PhasePhckrAudioProcessorEditorFX(
        PhasePhckrAudioProcessorFX&
     );
    ~PhasePhckrAudioProcessorEditorFX();

    void paint (Graphics&) override;
    void resized() override;

    friend PhasePhckrParameters;

private:
    PhasePhckrAudioProcessorFX& processor;

    PhasePhckrScope outputScopeL;
    PhasePhckrScope outputScopeR;
    PhasePhckrXYScope outputScopeXY;

    TabbedComponent mainFrame;
    PhasePhckrGrid scopeGrid;
    PhasePhckrGrid performGrid;

//    FileBrowserPanel fileBrowserPanel;

    PatchEditor effectEditor;

#if INTERCEPT_STD_STREAMS
    InterceptStringStream coutIntercept;
    InterceptStringStream cerrIntercept;
    TextEditor coutView;
    TextEditor cerrView;
    PhasePhckrGrid debugTab;
    LambdaTimer* debugViewUpdateTimer;
#endif

    vector<ParameterKnob *> parameterKnobs;
    LambdaTimer guiUpdateTimer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhasePhckrAudioProcessorEditorFX)
};


#endif  // PLUGINEDITOR_H_INCLUDED
