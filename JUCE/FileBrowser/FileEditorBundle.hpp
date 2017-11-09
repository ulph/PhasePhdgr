#pragma once

class FileEditorBundle : public Component
{
private:
    Label titleLabel;
    DirectoryContentsList watcher;
    FileListComponent list;
    PhasePhckrFileStuff::StupidFileBrowserListener listener;
public:
    FileEditorBundle(const string& name, const File& directory, TimeSliceThread& watchThread, StupidCallBack callBack)
        : watcher(PhasePhckrFileStuff::getFilter(), watchThread)
        , list(watcher)
        , listener(callBack)
        , titleLabel(String(), name)
    {
        list.addListener(&listener);
        watcher.setDirectory(directory, true, true);

        addAndMakeVisible(titleLabel);
        addAndMakeVisible(list);

        _stylize(&titleLabel); titleLabel.setJustificationType(Justification::left);
        _stylize(&list);

        resized();
    }

    void resized() override {
        const int rowHeightPx = 30;

        auto bounds = getBounds();

        titleLabel.setBounds(0, 0, bounds.getX()*0.5, rowHeightPx);
        list.setBounds(0, rowHeightPx, bounds.getX(), bounds.getY() - rowHeightPx);

    }

};