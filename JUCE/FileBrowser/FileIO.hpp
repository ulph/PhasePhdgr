#pragma once

#include <phasephckr.hpp>
#include <phasephckr_json.hpp>

#include "JuceHeader.h"
#include "Utils.hpp"

#include <regex>
#include <string>

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

    struct ComponentFileLoader {
        PhasePhckr::ComponentRegister componentRegister;
        SubValue<PhasePhckr::ComponentRegister>& subComponentRegister;

        ComponentFileLoader(SubValue<PhasePhckr::ComponentRegister>& subComponentRegister_)
            : subComponentRegister(subComponentRegister_)
        {
        }

        void rescan() {
            Array<File> initialFiles;
            int res = componentsDir.findChildFiles(initialFiles, File::findFiles, true, "*.json");
            if (res == 0) return;
            for (int i = 0; i < initialFiles.size(); i++) {
                const auto &f = initialFiles[i];
                String p = f.getRelativePathFrom(componentsDir);
                string n = string(&PhasePhckr::componentMarker, 1) + p.dropLastCharacters(5).toUpperCase().toStdString(); // remove .json
                n = make_path_agnostic(n);
                string s = f.loadFileAsString().toStdString();
                try {
                    json j = json::parse(s.c_str());
                    PhasePhckr::ComponentDescriptor cd = j;
                    cd.cleanUp();
                    componentRegister.registerComponent(n, cd);
                }
                catch (const std::exception& e) {
                    (void)e;
                    continue;
                    assert(0);
                }
            }
            subComponentRegister.set(-1, componentRegister);
        }
    };

    File storeScoped(const File& path, const string& type, const json& body, bool dry_run);

}
