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

public:

    void initialize(PhasePhckrAudioProcessor * p);

    void refreshParameters();

    bool accessParameter(int index, PhasePhckrParameter ** param); // JUCE layer needs to couple to UI element
    size_t numberOfParameters();

    void swapParameterIndices(string a, string b); // via gui

    void setVoiceParametersHandleMap(const parameterHandleMap& pv){
        // from synth
        voiceParameters = pv;
        refreshParameters();
    }

    void setEffectParametersHandleMap(const parameterHandleMap& pv){
        // from synth
        effectParameters = pv;
        refreshParameters();
    }

    void visitHandleParameterValues(PhasePhckr::Synth* synth){
        // to synth
        for(const auto kv: parameterRouting){
            auto idx = kv.first;
            auto type = kv.second.first;
            auto handle = kv.second.second;
            auto p = floatParameters[idx];
            float value = p->range.convertFrom0to1(*p);
            switch(type){
            case VOICE:
                synth->handleVoiceParameter(handle, value);
                break;
            case EFFECT:
                synth->handleEffectParameter(handle, value);
                break;
            default:
                break;
            }
        }
    }

    vector<PhasePhckr::ParameterDescriptor> serialize(){
        // from patch serialization - convert to a struct with strings

        auto v = vector<PhasePhckr::ParameterDescriptor>();
        for(const auto &kv : parameterNames){
            auto param = floatParameters[kv.second];
            PhasePhckr::ParameterDescriptor p = {
                kv.first,
                kv.second,
                *param,
                param->range.start,
                param->range.end
            };
            v.emplace_back(p);
        }
        return v;
    }

    void deserialize(const vector<PhasePhckr::ParameterDescriptor>& pv){
        // from patch deserialization - convert from struct with strings

        // clear stuff
        for (const auto& fp : floatParameters) {
            fp->clearName();
            fp->setValueNotifyingHost(0.f); // I think this is the correct thing to do
        }

        // copy the values
        for (const auto& p : pv) {
            parameterNames[p.id] = p.index;
            auto param = floatParameters[p.index];
            param->range.start = p.min;
            param->range.end = p.max;
            param->setValueNotifyingHost(param->range.convertTo0to1(p.value));
            // we could also set name but updateParameters takes care of that
        }
    }

};
