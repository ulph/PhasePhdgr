#include "FileIO.hpp"

namespace PhasePhckrFileStuff {

    nlohmann::json loadJson(const File & f) {
        String s = f.loadFileAsString();
        return nlohmann::json::parse(s.toStdString().c_str());
    }

}
