#ifndef PHASEPHCKRGRID_H_INCLUDED
#define PHASEPHCKRGRID_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "PhasePhckr.h"

class PhasePhckrGrid : public Component
{
public:
    void paint (Graphics&) override;
    void resized() override;
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhasePhckrGrid)
};

#endif  // PHASEPHCKRGRID_H_INCLUDED
