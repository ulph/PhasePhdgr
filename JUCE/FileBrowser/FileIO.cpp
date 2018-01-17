#include "phasephckr_json.hpp"
#include "FileIO.hpp"

#include <algorithm>
#include <iostream>

using namespace std;

namespace PhasePhckrFileStuff {

    nlohmann::json loadJson(const File & f) {
        String s = f.loadFileAsString();
        return nlohmann::json::parse(s.toStdString().c_str());
    }

    void storeJson(File &f, const nlohmann::json& j) {
        f.replaceWithText(j.dump(2, 4, ' '));
    }

    void createFileIfNeeded(File &file, const nlohmann::json& j) {
        if (!file.exists()) {
            storeJson(file, j);
        }
        else {
            json j2 = loadJson(file);
            if (j != j2) {
                storeJson(file, j);
            }
        }
    }

    void createDirIfNeeded(File dir) {
        cout << "resource directory " + dir.getFullPathName();
        if (!dir.exists()) {
            auto res = dir.createDirectory();
            if (res.ok()) {
                cout << " created" << endl;
            }
            else {
                cout << " was not created: " << endl;
                cerr << res.getErrorMessage() << endl;
            }
        }
        else {
            cout << " exists" << endl;
        }

    }

    const String sep = File::getSeparatorString();

    const String factorySubDir = sep + "factory";

    void createLibraryDirectoriesIfNeeded() {
        createDirIfNeeded(rootDir);
        createDirIfNeeded(effectsDir);
        createDirIfNeeded(effectsDir.getFullPathName() + factorySubDir);
        createDirIfNeeded(voicesDir);
        createDirIfNeeded(voicesDir.getFullPathName() + factorySubDir);
        createDirIfNeeded(componentsDir);
        createDirIfNeeded(componentsDir.getFullPathName() + factorySubDir);
        createDirIfNeeded(presetsDir);
        createDirIfNeeded(presetsDir.getFullPathName() + factorySubDir);
    }

    File getInitialVoiceFile() {
        return voicesDir.getFullPathName() + factorySubDir + sep + "_init.json";
    }

    File getInitialEffectFile() {
        return effectsDir.getFullPathName() + factorySubDir + sep + "_init.json";
    }

    void createInitialUserLibrary(const PhasePhckr::ComponentRegister& cr) {
        // TODO, control this

        createLibraryDirectoriesIfNeeded();

        // load init patches and dump to disk
        auto vf = getInitialVoiceFile();
        auto ef = getInitialEffectFile();
        const auto& vp = PhasePhckr::getExampleVoiceChain();
        const auto& ep = PhasePhckr::getExampleEffectChain();
        createFileIfNeeded(vf, vp);
        createFileIfNeeded(ef, ep);

        // dump all factory components to disk
        for (const auto &kv : cr.all()) {
            const auto &type = kv.first;
            const auto &body = kv.second;
            File cmp = componentsDir.getFullPathName() + factorySubDir + sep + type.substr(1) + ".json";
            storeJson(cmp, body);
        }
    }

    string make_path_agnostic(string& path) {
        string fromSep = String(File::getSeparatorString()).toStdString();
        auto re = regex("\\" + fromSep);
        auto newStr = regex_replace(path, re, string(1, PhasePhckr::scopeSeparator));
        return newStr;
    }

    string make_path_specific(string& path) {
        string toSep = String(File::getSeparatorString()).toStdString();
        auto re = regex("\\" + string(1, PhasePhckr::scopeSeparator));
        auto newStr = regex_replace(path, re, toSep);
        return newStr;
    }
}
