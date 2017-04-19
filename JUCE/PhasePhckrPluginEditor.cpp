/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PhasePhckrPluginProcessor.h"
#include "PhasePhckrPluginEditor.h"
#include "PhasePhckr.h"
#include <cstring>

//==============================================================================
PhasePhckrAudioProcessorEditor::PhasePhckrAudioProcessorEditor (PhasePhckrAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
    , outputScope(processor.getSynth()->getOutputScope())
    , voiceScope(processor.getSynth()->getVoiceScope())
    , mainFrame(TabbedButtonBar::TabsAtTop)
{
    setResizeLimits(128, 128, 1800, 1000);
    setConstrainer(nullptr);
    setResizable(true, true);
    setBoundsConstrained(Rectangle<int>(1800, 1000)); // slightly less than 1080p
    addAndMakeVisible(mainFrame);
    mainFrame.addTab("voice scope", Colours::black, &voiceScope, false);
    mainFrame.addTab("master scope", Colours::black, &outputScope, false);
    resized();
}

PhasePhckrAudioProcessorEditor::~PhasePhckrAudioProcessorEditor()
{
}

//==============================================================================
void PhasePhckrAudioProcessorEditor::paint (Graphics& g)
{
}

void PhasePhckrAudioProcessorEditor::resized()
{
    mainFrame.setBoundsRelative(0, 0, 1, 1);
    mainFrame.repaint(); // sometimes it does work reliably otherwise
    repaint();
}
