#pragma once

#include <map>
#include <vector>
#include <string>
#include <list>

#include "JuceHeader.h"

#include "phasephckr.hpp"

using namespace std;
using namespace PhasePhckr;

class PhasePhckrEditor;

class PhasePhckrParameter : public AudioParameterFloat {
private:
    int idx;
    SynthGraphType type = UNDEFINED;
    PatchParameterDescriptor pd;
    bool active = false;
    static string clearedName(int idx)
    {
        return to_string(idx / 8) + "_" + to_string(idx % 8);
    }
public:
    PhasePhckrParameter(int idx)
        : AudioParameterFloat(
            to_string(idx),
            clearedName(idx),
            0.0f,
            1.0f,
            0.0f
        )
        , idx(idx)
    {
        pd.id = clearedName(idx);
    }

    void reset() {
        active = false;
        type = UNDEFINED;
        pd.id = clearedName(idx);
        range.start = 0.f;
        range.end = 1.f;
        setValueNotifyingHost(this->range.convertTo0to1(0.f));
    }

    void initialize(SynthGraphType type_, PatchParameterDescriptor pd_) {
        active = true;
        type = type_;
        pd = pd_;
        range.start = pd_.v.min;
        range.end = pd_.v.max;
        setValueNotifyingHost(this->range.convertTo0to1(pd_.v.val));
    }

    const SynthGraphType& getType() {
        return type;
    }

    string getFullName() const {
        if (type == VOICE) return "v " + pd.id;
        else if (type == EFFECT) return "e " + pd.id;
        return pd.id;
    }

    virtual String getName(int maximumStringLength) const override {
        auto str = getFullName();
        return str.substr(0, maximumStringLength);
    }

    PatchParameterDescriptor getPatchParameterDescriptor() const {
        auto pdCopy = pd;
        pdCopy.v.val = *this;
        pdCopy.v.min = range.start;
        pdCopy.v.min = range.end;
        return pdCopy;
    }

    bool isActive() {
        return active;
    }

};

typedef pair<SynthGraphType, string> ParameterIdentifier;

class PhasePhckrParameters {

    static const int knobsPerBank = 8;
    static const int banksPerPage = 8;
    static const int numberOfPages = 8;

    vector<PhasePhckrParameter *> floatParameters; // the actual JUCE parameter, also holds the preset level name
    map<int, int> parameterRouting; // maps index of floatParameters to a handle
    ParameterHandleMap effectParameters;
    ParameterHandleMap voiceParameters;
    vector<PresetParameterDescriptor> presetParameters;
    simple_lock parameterLock;
    void updateParameters();

public:
    void initialize(AudioProcessor * p);
    template<class T> void initializeKnobs(T* e);
    bool accessParameter(int index, PhasePhckrParameter ** param); // JUCE layer needs to couple to UI element
    size_t numberOfParameters();
    void swapParameterIndices(int onto_idx, int dropped_idx); // via gui
    void setParametersHandleMap(SynthGraphType type, const ParameterHandleMap& pv);
    void visitHandleParameterValues(Synth* synth);
    void visitHandleParameterValues(Effect* effect);
    vector<PresetParameterDescriptor> serialize();
    void deserialize(const vector<PresetParameterDescriptor>& pv);
};

template<class T> void PhasePhckrParameters::initializeKnobs(T * e) {
    for (int i = 0; i<numberOfParameters(); i++) {
        PhasePhckrParameter* p = nullptr;
        if (accessParameter(i, &p)) {
            auto knob = new ParameterKnob(p,
                [this](int onto_idx, int dropped_idx) {
                    swapParameterIndices(onto_idx, dropped_idx);
                }
            );
            e->parameterKnobs.push_back(knob);
            e->performGrid.addComponent(knob);
        }
    }
}
