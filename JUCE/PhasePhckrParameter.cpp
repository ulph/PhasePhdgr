#include "PhasePhckrParameter.hpp"
#include "PhasePhckrPluginEditor.h"

#include <list>

using namespace std;

void PhasePhckrParameters::initialize(AudioProcessor * p){
    // only call once right after constructor or shit hits the fan
    for (int i = 0; i < 8*16; i++) {
        auto knb_ptr = new PhasePhckrParameter(i);
        floatParameters.push_back(knb_ptr);
        p->addParameter(knb_ptr);
    }
}

void PhasePhckrParameters::updateParameters()
{
    // clear the floatParameters
    for (auto fp : floatParameters) {
        fp->reset();
    }

    // clear the routing
    parameterRouting.clear();

    // build a map of valid parameter names to their default values, ranges and handles
    map<ParameterIdentifier, const PatchParameterDescriptor*> validParameters;
    map<ParameterIdentifier, int> validParametersHandles;
    for (const auto& kv : effectParameters) {
        ParameterIdentifier tid(EFFECT, kv.second.id);
        validParameters[tid] = &kv.second;
        validParametersHandles[tid] = kv.first;
    }
    for (const auto& kv : voiceParameters) {
        ParameterIdentifier tid(VOICE, kv.second.id);
        validParameters[tid] = &kv.second;
        validParametersHandles[tid] = kv.first;
    }

    // prune any invalid preset parameters
    set<ParameterIdentifier> validPresetParams; // and build a convinience set for checking existance
    set<int> occupiedParamIndexes;

    auto it = presetParameters.begin();
    while(it != presetParameters.end()){
        auto& ppd = *it;
        ParameterIdentifier tid(ppd.type, ppd.p.id);
        if (!validParameters.count(tid)
            || validPresetParams.count(tid)
            || occupiedParamIndexes.count(ppd.index)
            || ppd.index < 0
            || ppd.index >= floatParameters.size()
        ) {
            it = presetParameters.erase(it);
        }
        else {
            validPresetParams.insert(tid);
            occupiedParamIndexes.insert(ppd.index);
            it++;
        }
    }

    // for any parameters missing on preset, copy over from validParameters and find an index
    int firstValidSlot = 0;
    for (const auto& kv : validParameters) {
        if (!validPresetParams.count(kv.first)) {
            while (occupiedParamIndexes.count(firstValidSlot)) firstValidSlot++;
            if (firstValidSlot >= floatParameters.size()) return; // TODO whee oo whee too many params! notify the user!
            occupiedParamIndexes.insert(firstValidSlot);
            PresetParameterDescriptor ppd;
            ppd.index = firstValidSlot;
            ppd.type = kv.first.first;
            ppd.p = *kv.second;
            presetParameters.emplace_back(ppd);
        }
    }

    // populate the floatParameters and build the routing
    for (const auto& ppd : presetParameters) {
        ParameterIdentifier tid(ppd.type, ppd.p.id);
        assert(validParametersHandles.count(tid));
        floatParameters[ppd.index]->initialize(ppd.type, ppd.p);
        parameterRouting[ppd.index] = validParametersHandles.at(tid);
    }
    
}

bool PhasePhckrParameters::accessParameter(int index, PhasePhckrParameter ** param) {
    // potentially unsafe hack
    if (index >= numberOfParameters()) return false;
    *param = floatParameters[index];
    return true;
}

size_t PhasePhckrParameters::numberOfParameters() {
    return floatParameters.size();
}

void PhasePhckrParameters::swapParameterIndices(int a_idx, int b_idx) {

    if (a_idx == b_idx) return;
    if (a_idx < 0 || b_idx < 0) return;
    if (a_idx >= floatParameters.size() || b_idx >= floatParameters.size()) return;

    PresetParameterDescriptor * a = nullptr;
    PresetParameterDescriptor * b = nullptr;

    auto scoped_lock = parameterLock.make_scoped_lock();

    for (auto& ppd : presetParameters) {
        if (ppd.index == a_idx) a = &ppd;
        if (ppd.index == b_idx) b = &ppd;
    }

    if (a != nullptr && b != nullptr) {
        b->index = a_idx;
        a->index = b_idx;
        updateParameters();
    }

}

void PhasePhckrParameters::setParametersHandleMap(SynthGraphType type, const ParameterHandleMap& pv) {
    // from synth
    auto scoped_lock = parameterLock.make_scoped_lock();

    if (type == VOICE) {
        voiceParameters = pv;
        updateParameters();
    }
    else if (type == EFFECT) {
        effectParameters = pv;
        updateParameters();
    }
}

void PhasePhckrParameters::visitHandleParameterValues(PhasePhckr::Effect* effect) {
    auto scoped_lock = parameterLock.make_scoped_lock();

    for (const auto kv : parameterRouting) {
        auto idx = kv.first;
        auto fp = floatParameters.at(idx);
        auto type = fp->getType();
        auto handle = kv.second;
        float value = fp->range.convertFrom0to1(*fp);
        switch (type) {
        case EFFECT:
            effect->handleEffectParameter(handle, value);
            break;
        default:
            break;
        }
    }
}

void PhasePhckrParameters::visitHandleParameterValues(PhasePhckr::Synth* synth) {
    auto scoped_lock = parameterLock.make_scoped_lock();

    for (const auto kv : parameterRouting) {
        auto idx = kv.first;
        auto fp = floatParameters.at(idx);
        auto type = fp->getType();
        auto handle = kv.second;
        float value = fp->range.convertFrom0to1(*fp);
        switch (type) {
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

vector<PresetParameterDescriptor> PhasePhckrParameters::serialize() {
    // from patch serialization - convert to a struct with strings
    auto scoped_lock = parameterLock.make_scoped_lock();
    vector<PresetParameterDescriptor> pv;

    for (const auto& kv : parameterRouting) {
        auto fp = floatParameters.at(kv.first);
        auto type = fp->getType();
        auto index = kv.first;

        PresetParameterDescriptor pd;
        pd.index = kv.first;
        pd.type = type;
        pd.p = fp->getPatchParameterDescriptor();

        pv.push_back(pd);
    }

    return pv;
}

void PhasePhckrParameters::deserialize(const vector<PresetParameterDescriptor>& pv) {
    // from patch deserialization - convert from struct with strings
    auto scoped_lock = parameterLock.make_scoped_lock();

    presetParameters = pv;
    updateParameters();
}
