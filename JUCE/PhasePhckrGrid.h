#ifndef PHASEPHCKRGRID_H_INCLUDED
#define PHASEPHCKRGRID_H_INCLUDED

#include "JuceLibraryCode/JuceHeader.h"
#include "phasephckr.h"
#include <vector>

class PhasePhckrGrid : public Component
{
public:
    PhasePhckrGrid() : coloumnSizes({ 0.5f, 0.5f }) {}
    void paint (Graphics&) override;
    void resized() override;
    void addComponent(Component* component);
    void setColoumns(const std::vector<float> &coloumnSizes);
private:
    std::vector<float> coloumnSizes;
    std::vector<Component*> gridComponents;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhasePhckrGrid)
};

#endif  // PHASEPHCKRGRID_H_INCLUDED
