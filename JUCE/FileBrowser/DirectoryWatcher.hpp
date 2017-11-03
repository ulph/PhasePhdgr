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
