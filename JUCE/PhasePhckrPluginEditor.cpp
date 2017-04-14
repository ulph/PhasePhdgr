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
    size_t size = this->processor.getSynth()->getVoiceScope().copyBuffer(&scopeBuffer[0], sizeof(scopeBuffer)/sizeof(float));
    float size_y = (float)this->getHeight();
    float size_x = (float)this->getWidth();
    float yScale = size_y / (2.0f*1.0f + 0.25f);

    g.setColour(Colours::brown);
    g.drawHorizontalLine((int)(size_y*0.5f), 0.0f, size_x);
    g.drawHorizontalLine((int)(size_y*0.5f + yScale), 0.0f, size_x);
    g.drawHorizontalLine((int)(size_y*0.5f - yScale), 0.0f, size_x);

    g.setColour(Colours::yellow);
    if (size > 1 && size_x > 1) {
        float xScale = size_x / (float)(size-1);
        for (int i = 0; i < (size-1); ++i) {
            g.drawLine(
                i*xScale, 
                size_y*0.5f + yScale*scopeBuffer[i], 
                (i + 1)*xScale, 
                size_y*0.5f + yScale*scopeBuffer[i + 1], 
                1.f
            );
        }
    }
    repaint();
}

void PhasePhckrAudioProcessorEditor::resized()
{
}
