#pragma once

#include <map>
#include <vector>
#include <string>

#include "phasephckr.hpp"

#include "JuceHeader.h"

#include "PhasePhckrGrid.h"

using namespace std;
using namespace PhasePhckr;

typedef function<void(const string&, const MouseEvent &me)> ComponentClickedCallback;

class DocListModel : public ListBoxModel {
private:
    ComponentClickedCallback componentClicked;
    map<string, ModuleDoc> moduleDocs;
    vector<string> rows;
    TextEditor & docTextView;
    set<string> globalComponents;
    set<string> localComponents;
    void drawEntry(const string& key);
    string lastKey = "";
public:
    DocListModel(TextEditor & docTextView, const ComponentClickedCallback& cb);
    void setDocs(const map<string, ModuleDoc> & moduleDocs);
    void setGlobalComponents(const set<string>& globalComponents);
    void setLocalComponents(const set<string>& localComponents);
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
    DocView(const ComponentClickedCallback& cb=[](const string&, const MouseEvent&){}); // defaults to dummy callback
    void setGlobalComponents(const set<string>& globalComponents);
    void setLocalComponents(const set<string>& localComponents);
    void setDocs(const map<string, ModuleDoc> & moduleDocs);
    void resized() override;
};
