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
    string name;
    bool active;
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
        , name(clearedName(idx))
        , active(false)
    {
    }

    void reset() {
        range.start = 0.f;
        range.end = 1.f;
        name = clearedName(idx);
        active = false;
        setValueNotifyingHost(this->range.convertTo0to1(0.f));
    }

    void initialize(float newValue, float min, float max, string newName) {
        name = newName;
        range.start = min;
        range.end = max;
        active = true;
        setValueNotifyingHost(this->range.convertTo0to1(newValue));
    }

    const string &getFullName() {
        return name;
    }

    virtual String getName(int maximumStringLength) const override {
        return name.substr(0, maximumStringLength);
    }

    bool isActive() {
        return active;
    }

};

typedef pair<SynthGraphType, int> ParamterRoute;
typedef map<int, ParamterRoute> ParameterSlotToRouteMap;

class PhasePhckrParameters {
    vector<PhasePhckrParameter *> floatParameters; // the actual JUCE parameter, also holds the preset level name
    ParameterSlotToRouteMap parameterRouting; // maps index of floatParameters to a ROUTE (type and handle pair)
    ParameterHandleMap effectParameters;
    ParameterHandleMap voiceParameters;
    vector<PresetParameterDescriptor> presetParameters;
    simple_lock parameterLock;
    void updateParameters();

public:
    void initialize(AudioProcessor * p);
    void initializeKnobs(PhasePhckrEditor * e);
    bool accessParameter(int index, PhasePhckrParameter ** param); // JUCE layer needs to couple to UI element
    size_t numberOfParameters();
    void swapParameterIndices(int a_idx, int b_idx); // via gui
    void setParametersHandleMap(SynthGraphType type, const ParameterHandleMap& pv);
    void visitHandleParameterValues(Synth* synth);
    void visitHandleParameterValues(Effect* effect);
    vector<PresetParameterDescriptor> serialize();
    void deserialize(const vector<PresetParameterDescriptor>& pv);

};
