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
{
    setSize (1200, 680); // slightly less than 720p
    addAndMakeVisible(outputScope);
    addAndMakeVisible(voiceScope);
    resized();
}

PhasePhckrAudioProcessorEditor::~PhasePhckrAudioProcessorEditor()
{
}

//==============================================================================
void PhasePhckrAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll(Colours::black);
    g.setColour(Colours::white);
    g.drawHorizontalLine((int)(getHeight()*0.5f), 0.0f, (float)this->getWidth());
}

void PhasePhckrAudioProcessorEditor::resized()
{
    voiceScope.setBoundsRelative(0, 0, 1, 0.5);
    outputScope.setBoundsRelative(0, 0.5, 1, 0.5);
    repaint();
}
