#pragma once

#include <vector>
#include <string>
#include <map>

using namespace std;

namespace PhasePhckr {

    struct PadDescription {
        string name;
        string unit;
        float value;
    };

    struct ModuleDoc {
        string type;
        vector<PadDescription> inputs;
        vector<PadDescription> outputs;
        string docString;
    };

    class Doc {
    private:
        map<string, ModuleDoc> docs;
    public:
        Doc();
        const map<string, ModuleDoc> & get() const;
        void add(const ModuleDoc & d);
        static ModuleDoc makeBusModuleDoc(const vector<PadDescription>& ports, bool isInput);
    };
}
