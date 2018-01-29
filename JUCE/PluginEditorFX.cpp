#include <cstring>
#include <typeinfo>

#include <phasephckr.hpp>
#include <phasephckr_json.hpp>
#include "JuceHeader.h"

#include "PluginProcessorFX.h"
#include "PluginEditorFX.h"
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
    , mainFrame(
        [this](int i, const string& n) {
            if (!inited) return; // hack            
            if(n == "scopes") guiUpdateTimer.startTimer((int)(1.f / 60.f* 1000.f));
            else guiUpdateTimer.stopTimer();
            if (n == "parameters") parameterEditor.startTimer();
            else parameterEditor.stopTimer();
        }
    )
    , effectEditor(
        processor.subEffectChain,
        processor.subComponentRegister,
        c_effectChainInBus,
        c_effectChainOutBus,
        [this](const string& c, const map<string, ModulePosition>& l) { processor.updateLayout(c, l); }
    )
    , fileBrowserPanel(processor)
    , guiUpdateTimer(new function<void()>([this](){
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
    
    mainFrame.addTab("scopes", Colours::black, &scopePPGrid, false);
    scopePPGrid.addComponent(&inputScopeL);
    scopePPGrid.addComponent(&inputScopeXY);
    scopePPGrid.addComponent(&inputScopeR);
    scopePPGrid.addComponent(&outputScopeL);
    scopePPGrid.addComponent(&outputScopeXY);
    scopePPGrid.addComponent(&outputScopeR);
    scopePPGrid.setColoumns({0.33f, 0.33f, 0.33f});

    mainFrame.addTab("parameters", Colours::black, &parameterEditor, false);
    processor.parameters.initializeKnobs(parameterEditor);

    mainFrame.addTab("effect patch", Colours::black, &effectEditor, false);

    mainFrame.addTab("files", Colours::black, &fileBrowserPanel, false);

    processor.broadcastPatch();

    setLookAndFeel(&processor.lookAndFeel);

    resized();

    // triple hack
    inited = true;
    mainFrame.setCurrentTabIndex(1);
    mainFrame.setCurrentTabIndex(0);
}

PhasePhckrEditorFX::~PhasePhckrEditorFX()
{
    guiUpdateTimer.stopTimer();
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
