#pragma once

#include <map>
#include <vector>
#include <string>
#include <list>

#include "JuceHeader.h"

#include "phasephckr.hpp"

using namespace std;
using namespace PhasePhckr;

class Parameter : public AudioParameterFloat {
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
    Parameter(int idx)
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
        setValueNotifyingHost(0.f);
    }

    void initialize(SynthGraphType type_, PatchParameterDescriptor pd_) {
        active = true;
        type = type_;
        pd = pd_;
        range.start = pd_.v.min;
        range.end = pd_.v.max;
        setValueNotifyingHost(pd_.v.val);
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
        pdCopy.v.max = range.end;
        return pdCopy;
    }

    bool isActive() {
        return active;
    }

};
