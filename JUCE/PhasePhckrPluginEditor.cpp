#include "PhasePhckrPluginProcessor.h"
#include "PhasePhckrPluginEditor.h"
#include "PhasePhckr.h"
#include <cstring>

PhasePhckrAudioProcessorEditor::PhasePhckrAudioProcessorEditor (PhasePhckrAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
    , voiceScopeL(processor.getSynth()->getVoiceScope(0))
    , voiceScopeR(processor.getSynth()->getVoiceScope(1))
    , outputScopeL(processor.getSynth()->getOutputScope(0))
    , outputScopeR(processor.getSynth()->getOutputScope(1))
    , mainFrame(TabbedButtonBar::TabsAtTop)
{
    setResizeLimits(128, 128, 1800, 1000);
    setConstrainer(nullptr);
    setResizable(true, true);
    setBoundsConstrained(Rectangle<int>(1800, 1000)); // slightly less than 1080p
    addAndMakeVisible(mainFrame);
    mainFrame.addTab("scopes", Colours::black, &scopeGrid, false);
    scopeGrid.addComponent(&voiceScopeL);
    scopeGrid.addComponent(&voiceScopeR);
    scopeGrid.addComponent(&outputScopeL);
    scopeGrid.addComponent(&outputScopeR);
    mainFrame.addTab("edit", Colours::black, &editorGrid, false);
    editorGrid.addComponent(&voiceEditor);
    editorGrid.addComponent(&effectEditor);
    resized();
}

PhasePhckrAudioProcessorEditor::~PhasePhckrAudioProcessorEditor()
{
}

void PhasePhckrAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll(Colours::grey);
}

void PhasePhckrAudioProcessorEditor::resized()
{
    mainFrame.setBoundsRelative(0, 0, 1, 1);
    mainFrame.repaint(); // sometimes it does work reliably otherwise
    repaint();
}
