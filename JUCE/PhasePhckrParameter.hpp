#pragma once

class PhasePhckrParameters {
    vector<PhasePhckrParameter *> floatParameters;
    enum ApiType {
        VOICE,
        EFFECT
    };
    map<int, pair<ApiType, int>> parameterRouting;
    map<string, int> parameterNames;

    map<string, int> effectParameters;
    map<string, int> voiceParameters;

public:
    void updateParameters();

    // walk circles around the JUCE stuff a bit ...
    bool accessParameter(int index, PhasePhckrParameter ** param);
    size_t numberOfParameters();

    void swapParameterIndices(string a, string b);
};


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
