#include <cstring>
#include <typeinfo>

#include <phasephckr.hpp>
#include <phasephckr_json.hpp>
#include "JuceHeader.h"

#include "PhasePhckrPluginProcessor.h"
#include "PhasePhckrPluginEditor.h"
#include "DirectoryWatcher.hpp"

#include "FileIO.hpp"

using namespace PhasePhckrFileStuff;

using namespace std;

PhasePhckrEditor::PhasePhckrEditor(PhasePhckrProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
    , voiceScopeL(processor.getSynth()->getVoiceScope(0))
    , voiceScopeR(processor.getSynth()->getVoiceScope(1))
    , voiceScopeXY(processor.getSynth()->getVoiceScope(0), processor.getSynth()->getVoiceScope(1))
    , outputScopeL(processor.getSynth()->getEffectScope(0))
    , outputScopeR(processor.getSynth()->getEffectScope(1))
    , outputScopeXY(processor.getSynth()->getEffectScope(0), processor.getSynth()->getEffectScope(1))
    , mainFrame(TabbedButtonBar::TabsAtTop)

    , voiceEditor(
        processor.subVoiceChain,
        processor.subComponentRegister,
        c_voiceChainInBus,
        c_voiceChainOutBus
    )

    , effectEditor(
        processor.subEffectChain,
        processor.subComponentRegister,
        c_effectChainInBus,
        c_effectChainOutBus
    )

#if INTERCEPT_STD_STREAMS
    , coutIntercept(std::cout)
    , cerrIntercept(std::cerr)
#endif

    , fileBrowserPanel(processor)

    // TODO, tie the timer to the scopes view visibility
    , guiUpdateTimer(new function<void()>([this](){
        for(const auto &knob : parameterKnobs){
            knob->update();
        }
        voiceScopeL.repaint();
        voiceScopeR.repaint();
        voiceScopeXY.repaint();
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
    scopeGrid.addComponent(&voiceScopeL);
    scopeGrid.addComponent(&voiceScopeXY);
    scopeGrid.addComponent(&voiceScopeR);
    scopeGrid.addComponent(&outputScopeL);
    scopeGrid.addComponent(&outputScopeXY);
    scopeGrid.addComponent(&outputScopeR);
    scopeGrid.setColoumns({0.33f, 0.33f, 0.33f});

    mainFrame.addTab("preset parameters", Colours::black, &performGrid, false);
    performGrid.setColoumns({ 1.f ,1.f ,1.f ,1.f, 1.f, 1.f ,1.f ,1.f });

    processor.parameters.initializeKnobs(this);

    mainFrame.addTab("voice patch", Colours::black, &voiceEditor, false);
    mainFrame.addTab("effect patch", Colours::black, &effectEditor, false);

    mainFrame.addTab("files", Colours::black, &fileBrowserPanel, false);

#if INTERCEPT_STD_STREAMS
    mainFrame.addTab("debug", Colours::black, &debugTab, false);
    debugTab.addComponent(&coutView);
    debugTab.addComponent(&cerrView);
    coutView.setMultiLine(true, true);
    cerrView.setMultiLine(true, true);
    debugViewUpdateTimer = new LambdaTimer(new std::function<void()>([this](){
        std::string s;
        coutIntercept.readAll(s);
        if (s.size()) {
            coutView.insertTextAtCaret(s);
        }
        s.clear();
        cerrIntercept.readAll(s);
        if (s.size()) {
            cerrView.insertTextAtCaret(s);
        }
    }));
    debugViewUpdateTimer->startTimer(1000);
#endif

    processor.broadcastPatch();
    float fps = 60.f;
    guiUpdateTimer.startTimer((int)(1.f/fps* 1000.f));

    resized();
}

PhasePhckrEditor::~PhasePhckrEditor()
{
    guiUpdateTimer.stopTimer();
    for(const auto &knob : parameterKnobs){
        delete knob;
    }
#if INTERCEPT_STD_STREAMS
    debugViewUpdateTimer->stopTimer();
    delete debugViewUpdateTimer;
#endif
}

void PhasePhckrEditor::paint (Graphics& g)
{
    g.fillAll(Colours::black);
}

void PhasePhckrEditor::resized()
{
    mainFrame.setBoundsRelative(0.f, 0.f, 1.f, 1.f);
    repaint();
}
