#pragma once

#include <map>
#include <utility>
#include <functional>

#include "phasephdgr_json.hpp"

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

using namespace juce;

namespace PhasePhdgrFileStuff {

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

    typedef std::function<void(const DirectoryContentsList*)> StupidFileListCallBack;

    class StupidFileChangeListener : public ChangeListener{
    private:
        StupidFileListCallBack cb;
    public:
        StupidFileChangeListener(const StupidFileListCallBack& cb)
            : cb(cb){ }
        virtual void changeListenerCallback(ChangeBroadcaster *source){
            DirectoryContentsList* src = dynamic_cast<DirectoryContentsList *>(source);
            if(!src) return;
            cb(src);
        }
    };

}
