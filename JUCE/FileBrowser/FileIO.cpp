#include "phasephdgr_json.hpp"
#include "FileIO.hpp"

#include <algorithm>
#include <iostream>
#include <regex>
#include <string>

using namespace std;

namespace PhasePhdgrFileStuff {

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
        createDirIfNeeded(presetsDir);
        createDirIfNeeded(presetsDir.getFullPathName() + factorySubDir);

        createDirIfNeeded(sdkExtensionsDir);
    }

    File getInitialVoiceFile() {
        return voicesDir.getFullPathName() + factorySubDir + sep + "_init.json";
    }

    File getInitialEffectFile() {
        return effectsDir.getFullPathName() + factorySubDir + sep + "_init.json";
    }

    File storeScoped(const File& path, const string& bare_type, const json& body, bool dry_run) {
        // split prefix and name apart
        size_t found = bare_type.find_last_of(PhasePhdgr::scopeSeparator);
        string prefix = "";
        if (found != string::npos) {
            prefix = bare_type.substr(0, found);
        }
        string name = bare_type.substr(found + 1);

        // create folders if needed
        size_t start = 0;
        File exandedPath = path;
        while (start < prefix.size() && start != std::string::npos) {
            size_t stop = prefix.substr(start).find_first_of(PhasePhdgr::scopeSeparator);

            if (stop == std::string::npos) {
                exandedPath = exandedPath.getFullPathName() + sep + prefix.substr(start);
                if(!dry_run) createDirIfNeeded(exandedPath);
                break;
            }
            exandedPath = exandedPath.getFullPathName() + sep + prefix.substr(start, stop);
            if (!dry_run) createDirIfNeeded(exandedPath);

            start = start + stop + 1;
        }

        // store file
        File full_filename = exandedPath.getFullPathName() + sep + name + ".json";

        if (!dry_run) storeJson(full_filename, body);
        
        return full_filename;
    }

    void createInitialUserLibrary(const PhasePhdgr::ComponentRegister& cr) {
        // TODO, control this

        createLibraryDirectoriesIfNeeded();

        // load init patches and dump to disk
        auto vf = getInitialVoiceFile();
        auto ef = getInitialEffectFile();
        const auto& vp = PhasePhdgr::getExampleVoiceChain();
        const auto& ep = PhasePhdgr::getExampleEffectChain();
        createFileIfNeeded(vf, vp);
        createFileIfNeeded(ef, ep);

        // dump all factory components to disk
        for (const auto &kv : cr.all()) {
            const auto &type = kv.first;
            const auto &body = kv.second;
            if (!PhasePhdgr::componentTypeIsValid(type, true)) continue;
            storeScoped(componentsDir.getFullPathName(), type.substr(1), body, false);
        }
    }

    string make_path_agnostic(string& path) {
        string fromSep = String(File::getSeparatorString()).toStdString();
        auto re = regex("\\" + fromSep);
        auto newStr = regex_replace(path, re, string(1, PhasePhdgr::scopeSeparator));
        return newStr;
    }

    string make_path_specific(string& path) {
        string toSep = String(File::getSeparatorString()).toStdString();
        auto re = regex("\\" + string(1, PhasePhdgr::scopeSeparator));
        auto newStr = regex_replace(path, re, toSep);
        return newStr;
    }
}
