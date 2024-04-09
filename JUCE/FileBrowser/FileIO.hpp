#pragma once

#include <phasephdgr.hpp>
#include <phasephdgr_json.hpp>

#include "Utils.hpp"

#include <regex>
#include <string>

namespace PhasePhdgrFileStuff {
    nlohmann::json loadJson(const File & f);
    void storeJson(File &f, const nlohmann::json& j);

    const std::string phasePhdgrDirName = "phasephdgr";
    const std::string effectsDirName = "effect";
    const std::string voiceDirName = "voice";
    const std::string componentsDirName = "component";
    const std::string presetsDirName = "preset";

    const std::string sdkExtensionsDirName = "sdk_plugins";

    const File rootDir = File(
        File::getSpecialLocation(
            File::SpecialLocationType::userApplicationDataDirectory
        ).getFullPathName() + File::getSeparatorString() + phasePhdgrDirName
    );

    const File effectsDir = File(rootDir.getFullPathName() + File::getSeparatorString() + effectsDirName);
    const File voicesDir = File(rootDir.getFullPathName() + File::getSeparatorString() + voiceDirName);
    const File componentsDir = File(rootDir.getFullPathName() + File::getSeparatorString() + componentsDirName);
    const File presetsDir = File(rootDir.getFullPathName() + File::getSeparatorString() + presetsDirName);

    const File sdkExtensionsDir = File(rootDir.getFullPathName() + File::getSeparatorString() + sdkExtensionsDirName);

    void createDirIfNeeded(File dir);

    void createLibraryDirectoriesIfNeeded();

    File getInitialVoiceFile();

    File getInitialEffectFile();

    void createInitialUserLibrary(const PhasePhdgr::ComponentRegister& cr);

    string make_path_agnostic(string& path);

    string make_path_specific(string& path);

    struct ComponentFileLoader {
        PhasePhdgr::ComponentRegister componentRegister;
        SubValue<PhasePhdgr::ComponentRegister>& subComponentRegister;

        ComponentFileLoader(SubValue<PhasePhdgr::ComponentRegister>& subComponentRegister_)
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
                string n = string(&PhasePhdgr::componentMarker, 1) + p.dropLastCharacters(5).toUpperCase().toStdString(); // remove .json
                n = make_path_agnostic(n);
                string s = f.loadFileAsString().toStdString();
                try {
                    json j = json::parse(s.c_str());
                    PhasePhdgr::ComponentDescriptor cd = j;
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
