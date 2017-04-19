#ifndef PHASEPHKRSCOPE_H_INCLUDED
#define PHASEPHKRSCOPE_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "PhasePhckr.h"

class PhasePhckrScope : public Component
{
public:
    PhasePhckrScope(const PhasePhckr::Scope& source) : source(source) {}
    ~PhasePhckrScope() {}
    void paint (Graphics&) override;
    void resized() override;
private:
    const PhasePhckr::Scope& source;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhasePhckrScope)
};

#endif  // PHASEPHKRSCOPE_H_INCLUDED
