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


class ParameterKnob : public Component, public SliderListener, public DragAndDropTarget {
private:
    Slider slider;
    Label label;
    PhasePhckrParameter * parameter;
    const function<void(string, string)> swapParameterIndicesCallback;
public:
    ParameterKnob(PhasePhckrParameter * parameter, const function<void(string, string)> &swapParameterIndicesCallback)
        : parameter(parameter)
        , swapParameterIndicesCallback(swapParameterIndicesCallback)
    {
        slider.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(Slider::TextBoxRight, false, 50, 20);
        addAndMakeVisible(slider);
        addAndMakeVisible(label);
        label.setJustificationType(Justification::centred);
        slider.addListener(this);
        slider.addMouseListener(this, false);
        label.addMouseListener(this, false);
        resized();
    }

    void update() {
        label.setText(parameter->getName(64), sendNotificationAsync);
        slider.setRange(parameter->range.start, parameter->range.end);
        slider.setValue(*parameter, dontSendNotification);
        repaint();
    }

    void resized() override {
        label.setBoundsRelative(0.0f, 0.0f, 0.5f, 1.0f);
        slider.setBoundsRelative(0.5f, 0.0f, 0.5f, 1.0f);
        repaint();
    }

    void paint (Graphics& g) override {
        if(parameter->isActive()){
            g.fillAll(Colour(0xFF222222));
            label.setColour(Label::textColourId, Colours::lightgrey);
            slider.setColour(Slider::thumbColourId, Colours::lightgrey);
            slider.setColour(Slider::rotarySliderFillColourId, Colours::lightgrey);
            slider.setColour(Slider::rotarySliderOutlineColourId, Colours::black);
            slider.setColour(Slider::textBoxTextColourId, Colours::lightgrey);
            slider.setColour(Slider::textBoxOutlineColourId, Colours::lightgrey);
        }
        else{
            g.fillAll(Colour(0xFF0A0A0A));
            label.setColour(Label::textColourId, Colours::darkgrey);
            slider.setColour(Slider::thumbColourId, Colours::darkgrey);
            slider.setColour(Slider::rotarySliderFillColourId, Colours::darkgrey);
            slider.setColour(Slider::rotarySliderOutlineColourId, Colours::black);
            slider.setColour(Slider::textBoxTextColourId, Colours::darkgrey);
            slider.setColour(Slider::textBoxOutlineColourId, Colours::darkgrey);
        }
        const auto &r = getBounds();
        g.setColour(Colours::black);
        g.drawRect(0, 0, r.getWidth()-1, r.getHeight()-1, 1);
    }

    virtual void sliderDragStarted(Slider * slider) override {
        parameter->beginChangeGesture();
    }

    virtual void sliderDragEnded(Slider * slider) override {
        parameter->endChangeGesture();
    }

    virtual void sliderValueChanged(Slider * slider) override {
        parameter->setValueNotifyingHost(
            parameter->range.convertTo0to1((float)slider->getValue())
        );
    }

    virtual void mouseDown(const MouseEvent & event) override {
        if(dynamic_cast<Label*>(event.eventComponent)){
            if(event.mods.isLeftButtonDown()){
                auto drg = DragAndDropContainer::findParentDragContainerFor(this);
                var desc = label.getText();
                drg->startDragging(desc, this);
            }
        }
        else if(dynamic_cast<Slider*>(event.eventComponent)){
            if(event.mods.isRightButtonDown()){
                auto pm = PopupMenu();
                Label start("start", to_string(parameter->range.start));
                start.setEditable(true, true, false);
                pm.addItem(1, "start:");
                pm.addCustomItem(2, &start, 50, 20, false);
                Label end("end", to_string(parameter->range.end));
                end.setEditable(true, true, false);
                pm.addItem(3, "end:");
                pm.addCustomItem(4, &end, 50, 20, false);
                pm.show();
                float newStart = start.getText().getFloatValue();
                float newEnd = end.getText().getFloatValue();
                if(newEnd <= newStart){
                    return;
                }
                parameter->range.start = newStart;
                parameter->range.end = newEnd;
                update();
            }
        }
    }

    virtual bool isInterestedInDragSource (const SourceDetails &dragSourceDetails) override {
        return true;
    }

    virtual void itemDropped(const SourceDetails &dragSourceDetails) override {
        String desc = dragSourceDetails.description.toString();
        if(desc == label.getText()) return;
        swapParameterIndicesCallback(desc.toStdString(), label.getText().toStdString());
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
