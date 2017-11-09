#include "PhasePhckrParameter.hpp"
#include "PhasePhckrPluginProcessor.h"

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

void PhasePhckrParameters::updateOrFindNewParameters(list<parameterRoute>& newParams, string& firstNewName, const ParameterType& type, parameterHandleMap& newParameterNames, parameterRouteMap& newParameterRouting)
{
    string prefix = "";
    parameterHandleMap* existingParameterNames = nullptr;

    if (type == VOICE) {
        prefix += "v ";
        existingParameterNames = &voiceParameters;
    }
    else if (type == EFFECT) {
        prefix += "e ";
        existingParameterNames = &effectParameters;
    }

    if (!existingParameterNames) return;

    for (const auto& kv : *existingParameterNames) {
        string lbl = prefix + kv.first;
        auto route = make_pair(type, kv.second);
        auto it = parameterNames.find(lbl);
        if (it == parameterNames.end()) {
            newParams.push_back(make_pair(route, lbl));
            if (newParams.size() == 0) {
                firstNewName = lbl;
            }
        }
        else {
            floatParameters[it->second]->setName(lbl);
            newParameterRouting[it->second] = route;
            newParameterNames[lbl] = it->second;
        }
    }

}

void PhasePhckrParameters::refreshParameters()
{
    parameterHandleMap newParameterNames;
    parameterRouteMap newParameterRouting;

    // clear all the names, will get set back below
    for (const auto& p : parameterNames) {
        floatParameters[p.second]->clearName();
    }

    string firstNewName = "";
    list<parameterRoute> newParams; // urghhhh bookeeping thingy

    // find existing parameter (by name) and update it, or add to list of new parameters if not found
    updateOrFindNewParameters(newParams, firstNewName, VOICE, newParameterNames, newParameterRouting);
    updateOrFindNewParameters(newParams, firstNewName, EFFECT, newParameterNames, newParameterRouting);

    // special case - one new name and just one less new params -> a single rename
    if (newParams.size() == 1 && newParameterNames.size() == (parameterNames.size() - 1)) {
        for (const auto& kv : parameterNames) {
            if (!newParameterNames.count(kv.first)) {
                // found it!
                auto it = newParams.begin();
                while (it != newParams.end()) {
                    if (it->second == firstNewName) {
                        // found it also in newParams... apply and delete
                        newParameterRouting[kv.second] = it->first;
                        newParameterNames[firstNewName] = kv.second;
                        floatParameters[kv.second]->setName(firstNewName);
                        newParams.erase(it);
                        break;
                    }
                    it++;
                }
                break;
            }
        }
    }

    // for any new parameters, find first free slot and stick it there
    for (int i = 0; i<floatParameters.size(); i++) {
        if (newParams.size() == 0) break;
        if (newParameterRouting.count(i)) continue;
        auto p = newParams.front(); newParams.pop_front();
        newParameterRouting[i] = p.first;
        newParameterNames[p.second] = i;
        floatParameters[i]->setName(p.second);
    }

    if (newParams.size()) {
        cerr << "Warning - number of parameter modules larger than number allocated in plug-in!" << endl;
    }

    // replace the old route table and name book-keeping
    parameterNames = newParameterNames;
    parameterRouting = newParameterRouting;

    assert(parameterNames.size() == parameterRouting.size());
    for (const auto& p : parameterNames) {
        assert(parameterRouting.count(p.second));
    }

}


bool PhasePhckrParameters::accessParameter(int index, PhasePhckrParameter ** param) {
    if (index >= numberOfParameters()) return false;
    *param = floatParameters[index];
    return true;
}


size_t PhasePhckrParameters::numberOfParameters() {
    return floatParameters.size();
}


void PhasePhckrParameters::swapParameterIndices(string a, string b) {
    int a_idx = -1;
    int b_idx = -1;
    for (int i = 0; i<floatParameters.size(); i++) {
        if (floatParameters[i]->getName(64) == a) {
            assert(a_idx == -1);
            a_idx = i;
        }
        if (floatParameters[i]->getName(64) == b) {
            assert(b_idx == -1);
            b_idx = i;
        }
    }
    if (a_idx == b_idx) return;
    if (a_idx == -1 || b_idx == -1) return;
    if (!floatParameters[a_idx]->isActive() && !floatParameters[b_idx]->isActive()) return; // todo, should not even get this far...
    parameterNames[a] = b_idx;
    parameterNames[b] = a_idx;
    float a_val = *floatParameters[a_idx];
    float b_val = *floatParameters[b_idx];
    floatParameters[a_idx]->setValueNotifyingHost(b_val);
    floatParameters[b_idx]->setValueNotifyingHost(a_val);
    refreshParameters();
}
