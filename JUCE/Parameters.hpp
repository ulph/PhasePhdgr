#pragma once

#include "ParameterKnob.hpp"
#include "Parameter.hpp"
#include "PluginCommon.h"

typedef pair<SynthGraphType, string> ParameterIdentifier;

class Parameters {
    vector<Parameter *> floatParameters; // the actual JUCE parameter, also holds the preset level name
    map<int, int> parameterRouting; // maps index of floatParameters to a handle
    ParameterHandleMap effectParameters;
    ParameterHandleMap voiceParameters;
    vector<PresetParameterDescriptor> presetParameters;
    simple_lock parameterLock;
    void updateParameters(bool reset=false);

public:
    static const size_t knobsPerBank = 8;
    static const size_t banksPerPage = 8;
    static const size_t numberOfPages = 8;

    void initialize(AudioProcessor * p);
    void initializeKnobs(ParameterEditor& parameterEditor);
    bool accessParameter(size_t index, Parameter ** param); // JUCE layer needs to couple to UI element
    size_t numberOfParameters();
    void swapParameterIndices(size_t onto_idx, size_t dropped_idx); // via gui
    void setParametersHandleMap(SynthGraphType type, const ParameterHandleMap& pv);
    void visitHandleParameterValues(Synth* synth, PhasePhdgr::Effect* effect);
    void visitHandleParameterValues(Effect* effect);
    vector<PresetParameterDescriptor> serialize();
    void deserialize(const vector<PresetParameterDescriptor>& pv);
};
