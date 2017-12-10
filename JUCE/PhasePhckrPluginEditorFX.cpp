#include <cstring>
#include <typeinfo>

#include <phasephckr.hpp>
#include <phasephckr_json.hpp>
#include "JuceHeader.h"

#include "PhasePhckrPluginProcessorFX.h"
#include "PhasePhckrPluginEditorFX.h"
#include "DirectoryWatcher.hpp"

#include "FileIO.hpp"

using namespace PhasePhckrFileStuff;

using namespace std;

PhasePhckrEditorFX::PhasePhckrEditorFX(PhasePhckrProcessorFX& p)
    : AudioProcessorEditor(&p), processor (p)
    , inputScopeL(processor.getEffect()->getInputScope(0))
    , inputScopeR(processor.getEffect()->getInputScope(1))
    , inputScopeXY(processor.getEffect()->getInputScope(0), processor.getEffect()->getInputScope(1))
    , outputScopeL(processor.getEffect()->getEffectScope(0))
    , outputScopeR(processor.getEffect()->getEffectScope(1))
    , outputScopeXY(processor.getEffect()->getEffectScope(0), processor.getEffect()->getEffectScope(1))
    , mainFrame(TabbedButtonBar::TabsAtTop)
    , effectEditor(
        processor.subEffectChain,
        processor.subComponentRegister,
        c_effectChainInBus,
        c_effectChainOutBus
    )
    , fileBrowserPanel(processor)
    , guiUpdateTimer(new function<void()>([this](){
        for(const auto &knob : parameterKnobs){
            knob->update();
        }
        inputScopeL.repaint();
        inputScopeR.repaint();
        inputScopeXY.repaint();
        outputScopeL.repaint();
        outputScopeR.repaint();
        outputScopeXY.repaint();
    }))
{
    setResizeLimits(128, 128, 8000, 8000);
    setConstrainer(nullptr);
    setResizable(true, true);
    setBoundsConstrained(Rectangle<int>(1800, 1000)); // slightly less than 1080p
    addAndMakeVisible(mainFrame);
    
    mainFrame.addTab("scopes", Colours::black, &scopeGrid, false);
    scopeGrid.addComponent(&inputScopeL);
    scopeGrid.addComponent(&inputScopeXY);
    scopeGrid.addComponent(&inputScopeR);
    scopeGrid.addComponent(&outputScopeL);
    scopeGrid.addComponent(&outputScopeXY);
    scopeGrid.addComponent(&outputScopeR);
    scopeGrid.setColoumns({0.33f, 0.33f, 0.33f});

    mainFrame.addTab("preset parameters", Colours::black, &performGrid, false);
    performGrid.setColoumns({ 1.f ,1.f ,1.f ,1.f, 1.f, 1.f ,1.f ,1.f });

    processor.parameters.initializeKnobs(this);

    mainFrame.addTab("effect patch", Colours::black, &effectEditor, false);

    mainFrame.addTab("files", Colours::black, &fileBrowserPanel, false);

    processor.broadcastPatch();
    float fps = 30.f;
    guiUpdateTimer.startTimer((int)(1.f/fps* 1000.f));

    resized();
}

PhasePhckrEditorFX::~PhasePhckrEditorFX()
{
    guiUpdateTimer.stopTimer();
    for(const auto &knob : parameterKnobs){
        delete knob;
    }
}

void PhasePhckrEditorFX::paint (Graphics& g)
{
    g.fillAll(Colours::black);
}

void PhasePhckrEditorFX::resized()
{
    mainFrame.setBoundsRelative(0.f, 0.f, 1.f, 1.f);
    repaint();
}
