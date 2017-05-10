#ifndef DOCS_HPP
#define DOCS_HPP

#include <vector>
#include <string>
#include <map>

using namespace std;

struct PadDescription {
    std::string name;
    std::string unit;
    float value;
};

struct ModuleDoc {
    std::string type;
    std::vector<PadDescription> inputs;
    std::vector<PadDescription> outputs;
    std::string docString;
};

namespace PhasePhckr {
    class Doc {
    private:
        std::map<std::string, ModuleDoc> docs;
    public:
        Doc();
        const std::map<std::string, ModuleDoc> & get() const;
        void add(const ModuleDoc & d);
        static ModuleDoc makeBusModuleDoc(const vector<PadDescription>& ports, bool isInput);
    };
}



#endif
