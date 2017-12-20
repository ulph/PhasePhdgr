#pragma once

#include <phasephckr.hpp>
#include "JuceHeader.h"

class ScopeView : public Component
{
public:
    ScopeView(const PhasePhckr::Scope& source) : source(source) {}
    ~ScopeView() {}
    void paint (Graphics&) override;
    void resized() override;
private:
    const PhasePhckr::Scope& source;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScopeView)
};

class XYScopeView : public Component
{
public:
    XYScopeView(const PhasePhckr::Scope& sourceL, const PhasePhckr::Scope& sourceR) : sourceL(sourceL), sourceR(sourceR){}
    ~XYScopeView() {}
    void paint (Graphics&) override;
    void resized() override;
private:
    const PhasePhckr::Scope& sourceL;
    const PhasePhckr::Scope& sourceR;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XYScopeView)
};
