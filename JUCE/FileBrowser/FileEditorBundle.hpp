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
        auto boundsHeight = bounds.getHeight();

        float rowHeight = 0.01f;

        if (boundsHeight > 0) {
            rowHeight = (float)rowHeightPx / (float)boundsHeight;
        }

        titleLabel.setBoundsRelative(0.f, 0.f, 0.5f, rowHeight);
        list.setBoundsRelative(0.f, rowHeight, 1.f, 1.f - rowHeight);

    }

};