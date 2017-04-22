#ifndef PHASEPHCKRGRID_H_INCLUDED
#define PHASEPHCKRGRID_H_INCLUDED

#include "JuceLibraryCode/JuceHeader.h"
#include "phasephckr.h"
#include <vector>

class PhasePhckrGrid : public Component
{
public:
    PhasePhckrGrid() : numberOfColumns(2) {}
    void paint (Graphics&) override;
    void resized() override;
    void addComponent(Component* component);
    void setNumberOfColumns(int n);
private:
    int numberOfColumns;
    std::vector<Component*> gridComponents;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhasePhckrGrid)
};

#endif  // PHASEPHCKRGRID_H_INCLUDED
