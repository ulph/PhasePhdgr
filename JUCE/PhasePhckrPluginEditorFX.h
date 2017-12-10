#pragma once

#include <vector>

#include <phasephckr.hpp>
#include "JuceHeader.h"

#include "PhasePhckrPluginEditor.h"
#include "PhasePhckrPluginProcessorFX.h"
#include "PhasePhckrScope.h"
#include "DirectoryWatcher.hpp"
// #include "FileEditorBundle.hpp"
#include "GraphEditor.hpp"

#include "Utils.hpp"
#include "PatchEditor.hpp"

#include "ParameterKnob.hpp"

class PhasePhckrEditorFX  : public AudioProcessorEditor, public DragAndDropContainer
{
public:
    PhasePhckrEditorFX(
        PhasePhckrProcessorFX&
     );
    ~PhasePhckrEditorFX();

    void paint (Graphics&) override;
    void resized() override;

    friend PhasePhckrParameters;

private:
    PhasePhckrProcessorFX& processor;

    PhasePhckrScope outputScopeL;
    PhasePhckrScope outputScopeR;
    PhasePhckrXYScope outputScopeXY;

    TabbedComponent mainFrame;
    PhasePhckrGrid scopeGrid;
    PhasePhckrGrid performGrid;

//    FileBrowserPanel fileBrowserPanel;

    PatchEditor effectEditor;
    LambdaTimer guiUpdateTimer;

    vector<ParameterKnob *> parameterKnobs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhasePhckrEditorFX)
};
