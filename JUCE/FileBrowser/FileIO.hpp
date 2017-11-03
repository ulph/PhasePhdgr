#pragma once

#include "nlohmann/json.hpp"
#include "JuceHeader.h"

namespace PhasePhckrFileStuff {
    nlohmann::json loadJson(const File & f);
}
