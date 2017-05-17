#pragma once

#include "JuceHeader.h"
#include <map>
#include <utility>
#include <functional>

namespace PhasePhckrFileStuff {

    class StupidFileFilter : public FileFilter {
    public:
        StupidFileFilter(const String & desc) : FileFilter(desc) {}
        virtual bool isFileSuitable(const File &file) const override {
            std::string filestr = file.getFileExtension().toStdString();
            std::transform(filestr.begin(), filestr.end(), filestr.begin(), ::tolower);
            return filestr == std::string(".json");
        }
        virtual bool isDirectorySuitable(const File &file) const override {
            return true;
        }
    };

    const FileFilter * getFilter();

    const std::string phasePhkrDirName = "phasephkr";
    const std::string effectsDirName = "effects";
    const std::string voiceDirName = "voice";
    const std::string componentsDirName = "components";
    const std::string patchesDirName = "patches";

    const File rootDir = File(
        File::getSpecialLocation(
            File::SpecialLocationType::userApplicationDataDirectory
        ).getFullPathName() + File::separator + phasePhkrDirName
    );

    const File effectsDir = File(rootDir.getFullPathName() + File::separator + effectsDirName);
    const File voicesDir = File(rootDir.getFullPathName() + File::separator + voiceDirName);
    const File componentsDir = File(rootDir.getFullPathName() + File::separator + componentsDirName);
    const File patchesDir = File(rootDir.getFullPathName() + File::separator + patchesDirName);

    void createDirIfNeeded(File dir);

    typedef std::function<void(const File&)> StupidCallBack;

    class StupidFileBrowserListener : public FileBrowserListener {
    public:
        StupidFileBrowserListener() = delete;
        StupidFileBrowserListener(StupidCallBack callBack) : callBack(callBack) {}
        virtual void selectionChanged() {}
        virtual void fileClicked(const File &file, const MouseEvent &e) {
            callBack(file);
        }
        virtual void fileDoubleClicked(const File &file) {}
        virtual void browserRootChanged(const File &newRoot) {}
    private:
        StupidCallBack callBack;
    };

}
