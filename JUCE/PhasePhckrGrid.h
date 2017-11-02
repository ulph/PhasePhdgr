#ifndef PHASEPHCKRGRID_H_INCLUDED
#define PHASEPHCKRGRID_H_INCLUDED

#include <vector>
#include <iostream>

#include <phasephckr.hpp>
#include "JuceHeader.h"

using namespace std;

class PhasePhckrGrid : public Component
{
private:
    vector<float> coloumnSizes;
    vector<Component*> gridComponents;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhasePhckrGrid)

public:
    PhasePhckrGrid() : coloumnSizes({ 0.5f, 0.5f }) {}
    void paint (Graphics&) override;
    void resized() override;
    void addComponent(Component* component);
    void setColoumns(const vector<float> &coloumnSizes);
};

#endif  // PHASEPHCKRGRID_H_INCLUDED
