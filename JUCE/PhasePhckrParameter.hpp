#pragma once

#include <map>
#include <vector>
#include <string>
#include <list>

#include "JuceHeader.h"

#include "phasephckr.hpp"

using namespace std;

class PhasePhckrAudioProcessor;

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
    void clearName() {
        name = clearedName(idx);
        active = false;
        setValueNotifyingHost(this->range.convertTo0to1(*this));
    }
    void setName(string newName) {
        name = newName;
        active = true;
        setValueNotifyingHost(this->range.convertTo0to1(*this));
    }
    virtual String getName(int maximumStringLength) const override {
        return name.substr(0, maximumStringLength);
    }
    bool isActive() {
        return active;
    }
};


// TODO, review/refactor this stuff again to see if some of the maps and types and whatnot can be simplified/removed...

enum ParameterType {
    VOICE,
    EFFECT
};

typedef map<string, int> parameterHandleMap;
typedef map<int, pair<ParameterType, int>> parameterRouteMap;

typedef pair<pair<ParameterType, int>, string> parameterRoute; // ugh ...

class PhasePhckrParameters {
    vector<PhasePhckrParameter *> floatParameters;
    map<int, pair<ParameterType, int>> parameterRouting;
    map<string, int> parameterNames;

    parameterHandleMap effectParameters;
    parameterHandleMap voiceParameters;

    void updateOrFindNewParameters(list<parameterRoute>& newParams, string& firstNewName, const ParameterType& type, parameterHandleMap& newParameterNames, parameterRouteMap& newParameterRouting);

    std::atomic_flag parameterLock = ATOMIC_FLAG_INIT;

public:
    void initialize(PhasePhckrAudioProcessor * p);
    void refreshParameters();
    bool accessParameter(int index, PhasePhckrParameter ** param); // JUCE layer needs to couple to UI element
    size_t numberOfParameters();
    void swapParameterIndices(string a, string b); // via gui
    void setParametersHandleMap(ParameterType type, const parameterHandleMap& pv);
    void visitHandleParameterValues(PhasePhckr::Synth* synth);
    vector<PhasePhckr::ParameterDescriptor> serialize();
    void deserialize(const vector<PhasePhckr::ParameterDescriptor>& pv);

};
