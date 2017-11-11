#pragma once

#include <map>
#include <vector>
#include <string>

#include "phasephckr.hpp"
#include "JuceHeader.h"

#include "PhasePhckrGrid.h"

using namespace std;
using namespace PhasePhckr;

typedef function<void(const string&)> ComponentClickedCallback;

class DocListModel : public ListBoxModel {
private:
    ComponentClickedCallback componentClicked;
    map<string, ModuleDoc> moduleDocs;
    vector<string> rows;
    TextEditor & docTextView;
public:
    DocListModel(TextEditor & docTextView, const ComponentClickedCallback& cb);
    void setDocs(const map<string, ModuleDoc> & moduleDocs);
    virtual int getNumRows();
    virtual void paintListBoxItem(int rowNumber, Graphics &g, int width, int height, bool rowIsSelected);
    virtual void listBoxItemClicked(int row, const MouseEvent &);
    virtual var getDragSourceDescription (const SparseSet< int > &rowsToDescribe);
};


class DocView : public Component {
private:
    PhasePhckrGrid docGrid;
    DocListModel docListModel;
    TextEditor docTextView;
    ListBox docList;
public:
    DocView(const ComponentClickedCallback& cb=[](const string&){}); // defaults to dummy callback
    void setDocs(const map<string, ModuleDoc> & moduleDocs);
    void resized() override;
};
