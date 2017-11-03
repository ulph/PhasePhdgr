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
        f.replaceWithText(j.dump(2));
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

    void createLibraryDirectoriesIfNeeded() {
        createDirIfNeeded(rootDir);
        createDirIfNeeded(effectsDir);
        createDirIfNeeded(voicesDir);
        createDirIfNeeded(componentsDir);
        createDirIfNeeded(patchesDir);
    }

    File getInitialVoiceFile() {
        return voicesDir.getFullPathName() + File::getSeparatorString() + "_init.json";
    }

    File getInitialEffectFile() {
        return effectsDir.getFullPathName() + File::getSeparatorString() + "_init.json";
    }

    void createInitialUserLibrary(const PhasePhckr::ComponentRegister& cr) {
        // load init patches and dump to disk
        auto vf = getInitialVoiceFile();
        auto ef = getInitialEffectFile();
        storeJson(vf, PhasePhckr::getExampleVoiceChain());
        storeJson(ef, PhasePhckr::getExampleFxChain());

        // dump all factory components to disk
        for (const auto &kv : cr.all()) {
            const auto &type = kv.first;
            const auto &body = kv.second;
            File cmp = componentsDir.getFullPathName() + File::getSeparatorString() + type.substr(1) + ".json";
            storeJson(cmp, body);
        }
    }

}
