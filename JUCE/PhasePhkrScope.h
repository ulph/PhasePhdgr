#ifndef PHASEPHKRSCOPE_H_INCLUDED
#define PHASEPHKRSCOPE_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "PhasePhckr.h"

class PhasePhkrScope    : public Component
{
public:
    PhasePhkrScope(const PhasePhckr::Scope& source) : source(source) {}
    ~PhasePhkrScope() {}
    void paint (Graphics&) override;
    void resized() override;
private:
    const PhasePhckr::Scope& source;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhasePhkrScope)
};

#endif  // PHASEPHKRSCOPE_H_INCLUDED
