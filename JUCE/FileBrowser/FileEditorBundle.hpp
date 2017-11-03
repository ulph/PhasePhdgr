#pragma once

struct FileEditorBundle
{
    DirectoryContentsList watcher;
    FileListComponent list;
    PhasePhckrFileStuff::StupidFileBrowserListener listener;
public:
    FileEditorBundle(const File& directory, TimeSliceThread& watchThread, StupidCallBack callBack)
        : watcher(PhasePhckrFileStuff::getFilter(), watchThread)
        , list(watcher)
        , listener(callBack)
    {
        list.addListener(&listener);
        watcher.setDirectory(directory, true, true);
        _stylize(&list);
    }
};