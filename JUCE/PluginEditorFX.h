#pragma once

#include <vector>

#include <phasephckr.hpp>
#include "JuceHeader.h"

#include "PluginProcessorFX.h"
#include "ScopeView.h"
#include "DirectoryWatcher.hpp"
#include "FileBrowserPanelFX.hpp"
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

    friend Parameters;

private:
    bool inited = false;

    PhasePhckrProcessorFX& processor;

    ScopeView inputScopeL;
    ScopeView inputScopeR;
    XYScopeView inputScopeXY;

    ScopeView outputScopeL;
    ScopeView outputScopeR;
    XYScopeView outputScopeXY;

    PPTabbedComponent mainFrame;
    PPGrid scopePPGrid;
    ParameterEditor parameterEditor;

    FileBrowserPanelFX fileBrowserPanel;

    PatchEditor effectEditor;
    LambdaTimer guiUpdateTimer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhasePhckrEditorFX)
};
