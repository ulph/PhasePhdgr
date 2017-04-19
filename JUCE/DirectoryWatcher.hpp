#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
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
        virtual bool isDirectorySuitable(const File &file) const {
            return true;
        }
    };

    TimeSliceThread & getThread();
    const FileFilter * getFilter();

    const std::string phasePhkrDirName = "phasephkr";
    const std::string effectsDirName = "effects";
    const std::string voiceDirName = "voice";
    const std::string componentsDirName = "components";

    const File rootDir = File(
        File::getSpecialLocation(
            File::SpecialLocationType::userApplicationDataDirectory
        ).getFullPathName() + File::separator + phasePhkrDirName
    );

    const File effectsDir = File(rootDir.getFullPathName() + File::separator + effectsDirName);
    const File voicesDir = File(rootDir.getFullPathName() + File::separator + voiceDirName);
    const File componentsDir = File(rootDir.getFullPathName() + File::separator + componentsDirName);

    void createDirIfNeeded(File dir);

    typedef std::pair < int64_t, int64_t> FileInfo;
    typedef std::map<std::string, FileInfo> DirInfo;
    typedef std::function<void(std::vector<File>)> CallBack;

}
