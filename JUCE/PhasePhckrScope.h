#ifndef PHASEPHKRSCOPE_H_INCLUDED
#define PHASEPHKRSCOPE_H_INCLUDED

#include "JuceHeader.h"
#include "phasephckr.h"

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

class PhasePhckrXYScope : public Component
{
public:
    PhasePhckrXYScope(const PhasePhckr::Scope& sourceL, const PhasePhckr::Scope& sourceR) : sourceL(sourceL), sourceR(sourceR){}
    ~PhasePhckrXYScope() {}
    void paint (Graphics&) override;
    void resized() override;
private:
    const PhasePhckr::Scope& sourceL;
    const PhasePhckr::Scope& sourceR;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhasePhckrXYScope)
};

#endif  // PHASEPHKRSCOPE_H_INCLUDED
