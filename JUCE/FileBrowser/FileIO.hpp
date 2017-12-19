#pragma once

#include <phasephckr.hpp>
#include <phasephckr_json.hpp>

#include "JuceHeader.h"

#include <regex>

namespace PhasePhckrFileStuff {
    nlohmann::json loadJson(const File & f);
    void storeJson(File &f, const nlohmann::json& j);

    const std::string phasePhckrDirName = "phasephckr";
    const std::string effectsDirName = "effect";
    const std::string voiceDirName = "voice";
    const std::string componentsDirName = "component";
    const std::string presetsDirName = "preset";

    const File rootDir = File(
        File::getSpecialLocation(
            File::SpecialLocationType::userApplicationDataDirectory
        ).getFullPathName() + File::getSeparatorString() + phasePhckrDirName
    );

    const File effectsDir = File(rootDir.getFullPathName() + File::getSeparatorString() + effectsDirName);
    const File voicesDir = File(rootDir.getFullPathName() + File::getSeparatorString() + voiceDirName);
    const File componentsDir = File(rootDir.getFullPathName() + File::getSeparatorString() + componentsDirName);
    const File presetsDir = File(rootDir.getFullPathName() + File::getSeparatorString() + presetsDirName);

    void createDirIfNeeded(File dir);

    void createLibraryDirectoriesIfNeeded();

    File getInitialVoiceFile();

    File getInitialEffectFile();

    void createInitialUserLibrary(const PhasePhckr::ComponentRegister& cr);

    string make_path_agnostic(string& path);

    string make_path_specific(string& path);

}
