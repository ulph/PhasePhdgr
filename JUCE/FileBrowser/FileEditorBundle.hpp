#pragma once

class FileEditorBundle : public Component
{
    Label title;
    DirectoryContentsList watcher;
    FileListComponent list;
    PhasePhckrFileStuff::StupidFileBrowserListener listener;
public:
    FileEditorBundle(const string& name, const File& directory, TimeSliceThread& watchThread, StupidCallBack callBack)
        : watcher(PhasePhckrFileStuff::getFilter(), watchThread)
        , list(watcher)
        , listener(callBack)
        , title(name, name)
    {
        list.addListener(&listener);
        watcher.setDirectory(directory, true, true);
        addAndMakeVisible(title);
        addAndMakeVisible(list);
        _stylize(&list);
        resized();
    }

    void resized() override {
        const int rowHeightPx = 20;

        auto bounds = getBounds();

        title.setBounds(0, 0, bounds.getX()*0.5, rowHeightPx);
        list.setBounds(0, rowHeightPx, bounds.getX(), bounds.getY() - rowHeightPx);
    }

};