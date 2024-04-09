#pragma once

#include <phasephdgr.hpp>

#include <juce_audio_plugin_client/juce_audio_plugin_client.h>

using namespace juce;

class ScopeI : public Component {
    //
};

class ScopeView : public ScopeI
{
public:
    ScopeView(const PhasePhdgr::Scope& source) : source(source) {}
    ~ScopeView() {}
    void paint (Graphics&) override;
    void resized() override;
private:
    const PhasePhdgr::Scope& source;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScopeView)
};

class XYScopeView : public ScopeI
{
public:
    XYScopeView(const PhasePhdgr::Scope& sourceL, const PhasePhdgr::Scope& sourceR) : sourceL(sourceL), sourceR(sourceR){}
    ~XYScopeView() {}
    void paint (Graphics&) override;
    void resized() override;
private:
    const PhasePhdgr::Scope& sourceL;
    const PhasePhdgr::Scope& sourceR;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XYScopeView)
};
