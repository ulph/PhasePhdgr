#pragma once

#include <vector>
#include <iostream>

#include <phasephckr.hpp>
#include "JuceHeader.h"

using namespace std;

class PPGrid : public Component
{
private:
    vector<float> coloumnSizes;
    vector<Component*> components;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PPGrid)

public:
    PPGrid() : coloumnSizes({ 0.5f, 0.5f }) {}
    virtual void paint (Graphics&) override;
    virtual void resized() override;
    virtual void addComponent(Component* component);
    virtual void setColoumns(const vector<float> &coloumnSizes);
};

class PPGGrid : public GroupComponent {
private:
    PPGrid grid;
public:
    PPGGrid();
    virtual void resized() override;
    void addComponent(Component* component);
    void setColoumns(const vector<float> &coloumnSizes);
};