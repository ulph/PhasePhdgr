#pragma once

#include <phasephckr.hpp>

#include <juce_audio_plugin_client/juce_audio_plugin_client.h>

using namespace juce;

class ScopeI : public Component {
    //
};

class ScopeView : public ScopeI
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

class XYScopeView : public ScopeI
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
