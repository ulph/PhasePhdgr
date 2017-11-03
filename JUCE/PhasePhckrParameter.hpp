#pragma once

#include <map>
#include <vector>
#include <string>

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


typedef map<string, int> parameterMapping;


class PhasePhckrParameters {
    vector<PhasePhckrParameter *> floatParameters;
    enum ApiType {
        VOICE,
        EFFECT
    };
    map<int, pair<ApiType, int>> parameterRouting;
    map<string, int> parameterNames;

    parameterMapping effectParameters;
    parameterMapping voiceParameters;

public:

    void initialize(PhasePhckrAudioProcessor * p);

    void updateParameters();

    // walk circles around the JUCE stuff a bit ...
    bool accessParameter(int index, PhasePhckrParameter ** param);
    size_t numberOfParameters();

    void swapParameterIndices(string a, string b);

    void setVoiceParameters(const parameterMapping& pv){
        voiceParameters = pv;
        updateParameters();
    }

    void setEffectParameters(const parameterMapping& pv){
        effectParameters = pv;
        updateParameters();
    }

    void sendParameters(PhasePhckr::Synth* synth){
        for(const auto kv: parameterRouting){
            auto idx = kv.first;
            auto type = kv.second.first;
            auto handle = kv.second.second;
            auto p = floatParameters[idx];
            float value = p->range.convertFrom0to1(*p);
            switch(type){
            case VOICE:
                synth->setVoiceParameter(handle, value);
                break;
            case EFFECT:
                synth->setFxParameter(handle, value);
                break;
            default:
                break;
            }
        }
    }

    vector<PhasePhckr::ParameterDescriptor> serialize(){
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

        // also, update??
    }



};
