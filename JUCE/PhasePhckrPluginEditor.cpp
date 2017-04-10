/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PhasePhckrPluginProcessor.h"
#include "PhasePhckrPluginEditor.h"
#include "PhasePhckr.h"


//==============================================================================
PhasePhckrAudioProcessorEditor::PhasePhckrAudioProcessorEditor (PhasePhckrAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (1200, 680); // slightly less than 720p
}

PhasePhckrAudioProcessorEditor::~PhasePhckrAudioProcessorEditor()
{
}

//==============================================================================
void PhasePhckrAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::black);
    const PhasePhckr::Synth &synth = this->processor.getSynth();
    size_t size = synth.getScopeBuffer(&scopeBuffer[0], 1024);
    float size_y = this->getHeight();
    float size_x = this->getWidth();
    float yScale = size_y * 0.25;

    g.setColour(Colours::brown);
    g.drawHorizontalLine(size_y*0.5, 0, size_x);
    g.drawHorizontalLine(size_y*0.5 + yScale, 0, size_x);
    g.drawHorizontalLine(size_y*0.5 - yScale, 0, size_x);

    g.setColour(Colours::yellow);
    if (size > 1 && size_x > 1) {
        float xScale = size_x / (float)(size-1);
        for (int i = 0; i < (size-1); ++i) {
            g.drawLine(
                i*xScale, 
                size_y*0.5 + yScale*scopeBuffer[i], 
                (i + 1)*xScale, 
                size_y*0.5 + yScale*scopeBuffer[i + 1], 
                1.f
            );
        }
    }
    repaint();
}

void PhasePhckrAudioProcessorEditor::resized()
{
}
