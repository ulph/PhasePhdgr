#include "Parameter.hpp"
#include "PluginEditor.h"

#include <list>

using namespace std;

void Parameters::initialize(AudioProcessor * p){
    // only call once right after constructor or shit hits the fan
    for (int i = 0; i < knobsPerBank*banksPerPage*numberOfPages; i++) {
        auto knb_ptr = new Parameter(i);
        floatParameters.push_back(knb_ptr);
        p->addParameter(knb_ptr);
    }
}

void Parameters::updateParameters(bool reset)
{
    // clear the routing
    parameterRouting.clear();

    // reset params if instructed or empty
    auto noParams = effectParameters.size() == 0 && voiceParameters.size() == 0;
    if (reset || noParams) {
        for (auto fp : floatParameters) fp->reset();
    }
    if (noParams) return;

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

    // retain any valid floatParameter, clear any dangling or dupes
    map<ParameterIdentifier, int> validParameterIndices;
    for (auto fp : floatParameters) {
        if (!fp->isActive()) continue;
        ParameterIdentifier tid(fp->getType(), fp->getPatchParameterDescriptor().id);
        if(!validParameters.count(tid) || validParameterIndices.count(tid)) fp->reset();
        else validParameterIndices[tid] = fp->getParameterIndex();
    }

    // find any valid preset parameter
    map<ParameterIdentifier, int> validPresetParameters; // and build a convinience set for checking existance
    for (int i = 0; i < presetParameters.size(); ++i) {
        const auto& p = presetParameters.at(i);
        ParameterIdentifier tid(p.type, p.p.id);
        if (validParameters.count(tid)) validPresetParameters[tid] = i;
    }

    // update floatParameters and parameterRouting
    int firstFreeSlot = 0;
    for (const auto& kv : validParameters) {
        const auto& tid = kv.first;

        // 0. find a slot
        auto slot = -1;

        // a. check if this parameter allready exists
        if (validParameterIndices.count(tid)) {
            slot = validParameterIndices.at(tid);
            if (!reset) {
                // use existing parameter if not resetting
                parameterRouting[slot] = validParametersHandles.at(tid);
                continue; // skip out early, we don't want to set value!
            }
        }
        // b. check if exists in preset (state recall, file load)
        else if (validPresetParameters.count(tid)) {
            auto slot_ = presetParameters.at(validPresetParameters.at(tid)).index;
            if (slot_ < floatParameters.size() && !floatParameters.at(slot_)->isActive()) slot = slot_;
        }

        // c. could not find an existing slot, find next free (new param, missing/conflicting indices or otherwise broken state)
        if (slot < 0) {
            while (floatParameters.at(firstFreeSlot)->isActive()) {
                if (firstFreeSlot >= floatParameters.size()) return; // raise some exception ideally
                firstFreeSlot++;
            }
            slot = firstFreeSlot;
        }

        SynthGraphType type = tid.first;
        PatchParameterDescriptor pd;
        pd.id = tid.second;

        // initialize floatParameter
        // a. check if preset has
        // b. otherwise, use from patch
        if (validPresetParameters.count(tid)) {
            int i = validPresetParameters.at(tid);
            pd.v = presetParameters.at(i).p.v;
        }
        else {
            pd.v = kv.second->v;
        }
        floatParameters[slot]->initialize(type, pd);

        // 4. update routing
        parameterRouting[slot] = validParametersHandles.at(tid);
    }
}

bool Parameters::accessParameter(int index, Parameter ** param) {
    // potentially unsafe hack
    if (index >= numberOfParameters()) return false;
    *param = floatParameters[index];
    return true;
}

size_t Parameters::numberOfParameters() {
    return floatParameters.size();
}

void Parameters::swapParameterIndices(int onto_idx, int dropped_idx) {

    if (onto_idx == dropped_idx) return;
    if (onto_idx < 0 || dropped_idx < 0) return;
    if (onto_idx >= floatParameters.size() || dropped_idx >= floatParameters.size()) return;

    Parameter * a = nullptr;
    Parameter * b = nullptr;

    auto scoped_lock = parameterLock.make_scoped_lock();

    for (auto* fp : floatParameters) {
        auto index = fp->getParameterIndex();
        if (index == onto_idx) a = fp;
        if (index == dropped_idx) b = fp;
    }

    if (a != nullptr && b != nullptr) {
        auto aT = a->getType();
        auto aP = a->getPatchParameterDescriptor();

        auto bT = b->getType();
        auto bP = b->getPatchParameterDescriptor();

        a->initialize(bT, bP);
        b->initialize(aT, aP);

        updateParameters();
    }
    else if (b != nullptr) {
        auto bT = b->getType();
        auto bP = b->getPatchParameterDescriptor();
        b->reset();
        a->initialize(bT, bP);
        updateParameters();
    }

}

void Parameters::setParametersHandleMap(SynthGraphType type, const ParameterHandleMap& pv) {
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

void Parameters::visitHandleParameterValues(PhasePhckr::Effect* effect) {
    auto scoped_lock = parameterLock.make_scoped_lock();

    for (const auto kv : parameterRouting) {
        auto idx = kv.first;
        auto fp = floatParameters.at(idx);
        auto type = fp->getType();
        auto handle = kv.second;
        float value = *fp;
        switch (type) {
        case EFFECT:
            effect->handleEffectParameter(handle, value);
            break;
        default:
            break;
        }
    }
}

void Parameters::visitHandleParameterValues(PhasePhckr::Synth* synth) {
    auto scoped_lock = parameterLock.make_scoped_lock();

    for (const auto kv : parameterRouting) {
        auto idx = kv.first;
        auto fp = floatParameters.at(idx);
        auto type = fp->getType();
        auto handle = kv.second;
        float value = *fp;
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

vector<PresetParameterDescriptor> Parameters::serialize() {
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

void Parameters::deserialize(const vector<PresetParameterDescriptor>& pv) {
    // from patch deserialization - convert from struct with strings
    auto scoped_lock = parameterLock.make_scoped_lock();

    presetParameters = pv;
    updateParameters(true);
}