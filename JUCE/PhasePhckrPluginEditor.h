#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include "JuceHeader.h"
#include "PhasePhckrPluginProcessor.h"
#include "PhasePhckrScope.h"
#include "DirectoryWatcher.hpp"
#include "GraphEditor.hpp"
#include "docs.hpp"
#include <vector>
#include "Utils.hpp"
#include "PatchEditor.hpp"
#include "DirectoryWatcher.hpp"


class ParameterKnob : public Component, public SliderListener {
private:
    Slider slider;
    Label label;
    PhasePhckrParameter * parameter;
public:
    ParameterKnob(PhasePhckrParameter * parameter)
        : parameter(parameter)
    {
        slider.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
        slider.setRange(0.f, 1.f);
        slider.setTextBoxStyle(Slider::TextBoxRight, false, 40, 20);
        addAndMakeVisible(slider);
        addAndMakeVisible(label);
        slider.addListener(this);
        resized();
    }

    void update(){
        label.setText(parameter->getName(64), sendNotificationAsync);
        slider.setValue(*parameter);
    }

    void resized() override {
        label.setBoundsRelative(0.0f, 0.0f, 0.5f, 1.0f);
        slider.setBoundsRelative(0.5f, 0.0f, 0.5f, 1.0f);
        repaint();
    }

    void paint (Graphics& g) override {
        if(label.getText().isNotEmpty()){
            g.fillAll(Colours::darkgreen);
        }
        else{
            g.fillAll(Colours::darkgrey);
        }
    }

    virtual void sliderDragStarted(Slider * slider) override {
        parameter->beginChangeGesture();
    }

    virtual void sliderDragEnded(Slider * slider) override {
        parameter->endChangeGesture();
    }

    virtual void sliderValueChanged(Slider * slider) override {
        parameter->setValueNotifyingHost((float)slider->getValue());
    }

};

class PhasePhckrAudioProcessorEditor  : public AudioProcessorEditor, public DragAndDropContainer
{
public:
    PhasePhckrAudioProcessorEditor (
        PhasePhckrAudioProcessor&
     );
    ~PhasePhckrAudioProcessorEditor();

    void paint (Graphics&) override;
    void resized() override;

private:
    PhasePhckrAudioProcessor& processor;

    PhasePhckrScope voiceScopeL;
    PhasePhckrScope voiceScopeR;
    PhasePhckrXYScope voiceScopeXY;
    PhasePhckrScope outputScopeL;
    PhasePhckrScope outputScopeR;
    PhasePhckrXYScope outputScopeXY;

    TabbedComponent mainFrame;
    PhasePhckrGrid scopeGrid;
    PhasePhckrGrid performGrid;

    TimeSliceThread fileWatchThread;

    DirectoryContentsList voiceDirectoryWatcher;
    FileListComponent voiceDirectoryList;
    PhasePhckrFileStuff::StupidFileBrowserListener voiceListListener;
    PatchEditor voiceEditor;
    
    DirectoryContentsList effectDirectoryWatcher;
    FileListComponent effectDirectoryList;
    PhasePhckrFileStuff::StupidFileBrowserListener effectListListener;
    PatchEditor effectEditor;

    PhasePhckrGrid filesGrid;

#if INTERCEPT_STD_STREAMS
    InterceptStringStream coutIntercept;
    InterceptStringStream cerrIntercept;
    TextEditor coutView;
    TextEditor cerrView;
    PhasePhckrGrid debugTab;
    LambdaTimer* debugViewUpdateTimer;
#endif

    vector<ParameterKnob *> parameterKnobs;
    LambdaTimer parameterUpdateTimer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhasePhckrAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED
