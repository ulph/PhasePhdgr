#include "PhasePhckrParameter.hpp"
#include "PhasePhckrPluginProcessor.h"
#include "PhasePhckrPluginEditor.h"

#include <list>

using namespace std;


void PhasePhckrParameters::initialize(PhasePhckrAudioProcessor * p){
    // only call once right after constructor or shit hits the fan
    for (int i = 0; i < 8*16; i++) {
        auto knb_ptr = new PhasePhckrParameter(i);
        floatParameters.push_back(knb_ptr);
        p->addParameter(knb_ptr);
    }
}


void PhasePhckrParameters::initializeKnobs(PhasePhckrAudioProcessorEditor * e) {
    for (int i = 0; i<numberOfParameters(); i++) {
        PhasePhckrParameter* p = nullptr;
        if (accessParameter(i, &p)) {
            auto knob = new ParameterKnob(p,
                [this, e](int a, int b) {
                    swapParameterIndices(a, b);
                    e->guiUpdateTimer.timerCallback();
                }
            );
            e->parameterKnobs.push_back(knob);
            e->performGrid.addComponent(knob);
        }
    }
}

typedef pair<SynthGraphType, string> TID;

void PhasePhckrParameters::updateParameters()
{
    // clear the floatParameters
    for (auto fp : floatParameters) {
        fp->reset();
    }

    // clear the routing
    parameterRouting.clear();

    // build a map of valid parameter names to their default values, ranges and handles
    map<TID, const PatchParameterDescriptor*> validParameters;
    map<TID, int> validParametersHandles;
    for (const auto& kv : effectParameters) {
        TID tid = make_pair(EFFECT, kv.second.id);
        validParameters[tid] = &kv.second;
        validParametersHandles[tid] = kv.first;
    }
    for (const auto& kv : voiceParameters) {
        TID tid = make_pair(VOICE, kv.second.id);
        validParameters[tid] = &kv.second;
        validParametersHandles[tid] = kv.first;
    }

    // prune any invalid preset parameters
    set<TID> validPresetParams; // and build a convinience set for checking existance
    set<int> occupiedParamIndexes;

    auto it = presetParameters.begin();
    while(it != presetParameters.end()){
        auto& ppd = *it;
        TID tid = make_pair(ppd.p.type, ppd.p.id);
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
            ppd.p = *kv.second;
            presetParameters.emplace_back(ppd);
        }
    }

    // populate the floatParameters and build the routing
    for (const auto& ppd : presetParameters) {
        auto param = ppd.p;
        string name = ppd.p.type == VOICE ? "v: " : "e: ";
        name += param.id;
        TID tid = make_pair(param.type, param.id);
        assert(validParametersHandles.count(tid));

        floatParameters[ppd.index]->initialize(param.value, param.min, param.max, name);
        parameterRouting[ppd.index] = make_pair(param.type, validParametersHandles.at(tid));
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

    while (parameterLock.test_and_set(std::memory_order_acquire));

    for (auto& ppd : presetParameters) {
        if (ppd.index == a_idx) a = &ppd;
        if (ppd.index == b_idx) b = &ppd;
    }

    if (a != nullptr && b != nullptr) {
        b->index = a_idx;
        a->index = b_idx;
        updateParameters();
    }

    parameterLock.clear(std::memory_order_release);
}


void PhasePhckrParameters::setParametersHandleMap(SynthGraphType type, const ParameterHandleMap& pv) {
    // from synth
    if (type == VOICE) {
        while (parameterLock.test_and_set(std::memory_order_acquire));
        voiceParameters = pv;
        updateParameters();
        parameterLock.clear(std::memory_order_release);
    }
    else if (type == EFFECT) {
        while (parameterLock.test_and_set(std::memory_order_acquire));
        effectParameters = pv;
        updateParameters();
        parameterLock.clear(std::memory_order_release);
    }
}


void PhasePhckrParameters::visitHandleParameterValues(PhasePhckr::Synth* synth) {
    // to synth
    while (parameterLock.test_and_set(std::memory_order_acquire));

    for (const auto kv : parameterRouting) {
        auto idx = kv.first;
        auto route = kv.second;
        auto type = route.first;
        auto handle = route.second;
        auto p = floatParameters[idx];
        float value = p->range.convertFrom0to1(*p);
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

    parameterLock.clear(std::memory_order_release);

}


vector<PresetParameterDescriptor> PhasePhckrParameters::serialize() {
    // from patch serialization - convert to a struct with strings

    while (parameterLock.test_and_set(std::memory_order_acquire));

    auto pv = presetParameters;

    parameterLock.clear(std::memory_order_release);

    return pv;
}


void PhasePhckrParameters::deserialize(const vector<PresetParameterDescriptor>& pv) {
    // from patch deserialization - convert from struct with strings

    while (parameterLock.test_and_set(std::memory_order_acquire));

    presetParameters = pv;
    updateParameters();

    parameterLock.clear(std::memory_order_release);
}