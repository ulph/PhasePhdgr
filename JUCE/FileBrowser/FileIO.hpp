#pragma once

#include <phasephckr.hpp>
#include <phasephckr_json.hpp>

#include "JuceHeader.h"

namespace PhasePhckrFileStuff {
    nlohmann::json loadJson(const File & f);
    void storeJson(File &f, const nlohmann::json& j);

    const std::string phasePhkrDirName = "phasephkr";
    const std::string effectsDirName = "effects";
    const std::string voiceDirName = "voice";
    const std::string componentsDirName = "components";
    const std::string patchesDirName = "patches";

    const File rootDir = File(
        File::getSpecialLocation(
            File::SpecialLocationType::userApplicationDataDirectory
        ).getFullPathName() + File::getSeparatorString() + phasePhkrDirName
    );

    const File effectsDir = File(rootDir.getFullPathName() + File::getSeparatorString() + effectsDirName);
    const File voicesDir = File(rootDir.getFullPathName() + File::getSeparatorString() + voiceDirName);
    const File componentsDir = File(rootDir.getFullPathName() + File::getSeparatorString() + componentsDirName);
    const File patchesDir = File(rootDir.getFullPathName() + File::getSeparatorString() + patchesDirName);

    void createDirIfNeeded(File dir);

    void createLibraryDirectoriesIfNeeded();

    File getInitialVoiceFile();

    File getInitialEffectFile();

    void createInitialUserLibrary(const PhasePhckr::ComponentRegister& cr);

}
