#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include <vector>

#include <phasephckr.hpp>
#include "JuceHeader.h"

#include "PhasePhckrPluginProcessor.h"
#include "PhasePhckrScope.h"
#include "DirectoryWatcher.hpp"
#include "FileEditorBundle.hpp"
#include "GraphEditor.hpp"

#include "Utils.hpp"
#include "PatchEditor.hpp"

#include "ParameterKnob.hpp"


class PhasePhckrAudioProcessorEditor  : public AudioProcessorEditor, public DragAndDropContainer
{
public:
    PhasePhckrAudioProcessorEditor (
        PhasePhckrAudioProcessor&
     );
    ~PhasePhckrAudioProcessorEditor();

    void paint (Graphics&) override;
    void resized() override;

private:
    PhasePhckrAudioProcessor& processor;

    PhasePhckrScope voiceScopeL;
    PhasePhckrScope voiceScopeR;
    PhasePhckrXYScope voiceScopeXY;
    PhasePhckrScope outputScopeL;
    PhasePhckrScope outputScopeR;
    PhasePhckrXYScope outputScopeXY;

    TabbedComponent mainFrame;
    PhasePhckrGrid scopeGrid;
    PhasePhckrGrid performGrid;

    TimeSliceThread fileWatchThread;

    FileEditorBundle voiceFiles;
    FileEditorBundle effectFiles;
    FileEditorBundle patchFiles;
    FileEditorBundle componentFiles;

    PatchEditor voiceEditor;
    PatchEditor effectEditor;

    PhasePhckrGrid filesGrid;

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhasePhckrAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED
