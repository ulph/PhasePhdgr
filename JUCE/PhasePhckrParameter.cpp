#include "PhasePhckrParameter.hpp"
#include "PhasePhckrPluginEditor.h"

#include <list>

using namespace std;

void PhasePhckrParameters::initialize(AudioProcessor * p){
    // only call once right after constructor or shit hits the fan
    for (int i = 0; i < knobsPerBank*banksPerPage*numberOfPages; i++) {
        auto knb_ptr = new PhasePhckrParameter(i);
        floatParameters.push_back(knb_ptr);
        p->addParameter(knb_ptr);
    }
}

void PhasePhckrParameters::updateParameters(bool reset)
{
    if (reset) {
        for (auto fp : floatParameters) fp->reset();
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
        auto slot = 0;
        if (validParameterIndices.count(tid)) {
            slot = validParameterIndices.at(tid);
            if (!reset) {
                // use existing parameter if not resetting
                parameterRouting[slot] = validParametersHandles.at(tid);
                continue;
            }
        }
        else {
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
        // c. if not - assert (but we'll use default values implicitly)
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

bool PhasePhckrParameters::accessParameter(int index, PhasePhckrParameter ** param) {
    // potentially unsafe hack
    if (index >= numberOfParameters()) return false;
    *param = floatParameters[index];
    return true;
}

size_t PhasePhckrParameters::numberOfParameters() {
    return floatParameters.size();
}

void PhasePhckrParameters::swapParameterIndices(int onto_idx, int dropped_idx) {

    if (onto_idx == dropped_idx) return;
    if (onto_idx < 0 || dropped_idx < 0) return;
    if (onto_idx >= floatParameters.size() || dropped_idx >= floatParameters.size()) return;

    PhasePhckrParameter * a = nullptr;
    PhasePhckrParameter * b = nullptr;

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

void PhasePhckrParameters::visitHandleParameterValues(PhasePhckr::Synth* synth) {
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
    updateParameters(true);
}
