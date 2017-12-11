#pragma once

#include <atomic>

class ParameterKnob : public Component, public SliderListener, public DragAndDropTarget {
private:
    Slider slider;
    Label label;
    PhasePhckrParameter * parameter;
    float lastValue;
    int lastActivity = -1;
    String lastText;
    float lastMin;
    float lastMax;
    const function<void(int, int)> swapParameterIndicesCallback;
public:
    ParameterKnob(PhasePhckrParameter * parameter, const function<void(int, int)> &swapParameterIndicesCallback)
        : parameter(parameter)
        , swapParameterIndicesCallback(swapParameterIndicesCallback)
    {
        label.setColour(Label::textColourId, Colours::lightgrey);
        slider.setColour(Slider::thumbColourId, Colours::lightgrey);
        slider.setColour(Slider::rotarySliderFillColourId, Colours::lightgrey);
        slider.setColour(Slider::rotarySliderOutlineColourId, Colours::black);
        slider.setColour(Slider::textBoxTextColourId, Colours::lightgrey);
        slider.setColour(Slider::textBoxOutlineColourId, Colours::lightgrey);
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
        int newActivity = (int)parameter->isActive();
        if (newActivity != lastActivity) {
            if (newActivity) {
                slider.setVisible(true);
                label.setColour(Label::textColourId, Colours::lightgrey);
            }
            else {
                label.setColour(Label::textColourId, Colours::darkgrey);
                slider.setVisible(false);
            }
        }
        else if (!newActivity) return;

        float newMin = parameter->range.start;
        float newMax = parameter->range.end;
        if (lastMin != newMin || lastMax != newMax) {
            slider.setRange(newMin, newMax);
        }

        float newValue = *parameter;
        if (newValue != lastValue) {
            slider.setValue(newValue, dontSendNotification);
        }

        String newText = parameter->getName(64);
        if (lastText != newText) {
            label.setText(newText, sendNotificationAsync);
        }

        if(lastMin != newMin
        || lastMax != newMax
        || lastValue != newValue
        || lastActivity != newActivity
        || lastText != newText
        ){
            repaint();
        }

        lastMin = newMin;
        lastMax = newMax;
        lastValue = newValue;
        lastActivity = newActivity;
        lastText = newText;

    }

    void resized() override {
        label.setBoundsRelative(0.0f, 0.0f, 0.5f, 1.0f);
        slider.setBoundsRelative(0.5f, 0.0f, 0.5f, 1.0f);
        repaint();
    }

    void paint(Graphics& g) override {
        if (parameter->isActive()) {
            g.fillAll(Colour(0xFF222222));
        }
        else {
            g.fillAll(Colour(0xFF0A0A0A));
        }
        const auto &r = getBounds();
        g.setColour(Colours::black);
        g.drawRect(0, 0, r.getWidth() - 1, r.getHeight() - 1, 1);
    }

    virtual void sliderDragStarted(Slider * slider_) override {
        parameter->beginChangeGesture();
    }

    virtual void sliderDragEnded(Slider * slider_) override {
        parameter->endChangeGesture();
    }

    virtual void sliderValueChanged(Slider * slider_) override {
        parameter->setValueNotifyingHost(
            parameter->range.convertTo0to1((float)slider_->getValue())
        );
    }

    virtual void mouseDown(const MouseEvent & event) override {
        if (!this->parameter->isActive()) return; // deny interraction with inactive knobs

        if (dynamic_cast<Label*>(event.eventComponent)) {
            if (event.mods.isLeftButtonDown()) {
                auto drg = DragAndDropContainer::findParentDragContainerFor(this);
                var desc = this->parameter->getParameterIndex();
                drg->startDragging(desc, this);
            }
        }
        else if (dynamic_cast<Slider*>(event.eventComponent)) {
            if (event.mods.isRightButtonDown()) {
                auto pm = PopupMenu();
                Label start("start", to_string(parameter->range.start));
                start.setEditable(true, true, false);
                pm.addSectionHeader("min:");
                pm.addCustomItem(2, &start, 50, 20, false);
                Label end("end", to_string(parameter->range.end));
                end.setEditable(true, true, false);
                pm.addSectionHeader("max:");
                pm.addCustomItem(4, &end, 50, 20, false);
                pm.show();
                float newStart = start.getText().getFloatValue();
                float newEnd = end.getText().getFloatValue();
                if (newEnd <= newStart) {
                    return;
                }
                parameter->range.start = newStart;
                parameter->range.end = newEnd;
                update();
            }
        }
    }

    virtual bool isInterestedInDragSource(const SourceDetails &dragSourceDetails) override {
        return true;
    }

    virtual void itemDropped(const SourceDetails &dragSourceDetails) override {
        int this_idx = this->parameter->getParameterIndex();
        int other_idx = dragSourceDetails.description;
        swapParameterIndicesCallback(this_idx, other_idx);
    }

};

