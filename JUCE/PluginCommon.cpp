#pragma once

#include "PluginCommon.h"

void proccessHandleCommonMidi(Effect* effect) {

}

void handlePlayHead(Effect* effect, AudioPlayHead* playHead, const int blockSize, const float sampleRate, float& barPosition) {
    if (playHead == nullptr) return;
    AudioPlayHead::CurrentPositionInfo info;
    playHead->getCurrentPosition(info);
    effect->handleTimeSignature(info.timeSigNumerator, info.timeSigDenominator);
    effect->handleBPM((float)info.bpm);
    effect->handlePosition((float)info.ppqPosition);
    effect->handleTime((float)info.timeInSeconds);
    if (info.isPlaying) {
        barPosition = (float)info.ppqPosition - (float)info.ppqPositionOfLastBarStart;
    }
    else {
        auto timedelta = (float)blockSize / sampleRate;
        auto quarterdelta = timedelta * (float)info.bpm / 60.f;
        auto barLength = 4.f * (float)info.timeSigNumerator / (float)info.timeSigDenominator;
        barPosition += quarterdelta;
        barPosition = fmod(barPosition, barLength);
    }
    effect->handleBarPosition(barPosition);
}

ParameterEditor::ParameterEditor()
{
    addAndMakeVisible(pageTabs);
}

ParameterEditor::~ParameterEditor() {
    for (auto* p : pages) delete p;
}

void ParameterEditor::resized()
{
    pageTabs.setBoundsRelative(0.f, 0.0f, 1.f, 1.0f);
    repaint();
}

void ParameterEditor::addKnob(ParameterKnob* knob) {
    const int knobsPerPage = Parameters::knobsPerBank*Parameters::banksPerPage;
    if (0 == (knobCtr % knobsPerPage)) {
        auto* p = new PPGrid;
        p->setColoumns(rowLayout);
        pages.push_back(p);
        pageTabs.addTab(String(knobCtr) + "-" + String(knobCtr + knobsPerPage-1), Colours::black, p, false);
    }
    pages.back()->addComponent(knob);
    knobCtr++;
}