#pragma once

#include <map>
#include <vector>
#include <string>

#include "phasephckr.hpp"
#include "JuceHeader.h"

#include "PhasePhckrGrid.h"

using namespace std;
using namespace PhasePhckr;


class DocListModel : public ListBoxModel {
private:
    map<string, ModuleDoc> moduleDocs;
    vector<string> rows;
    TextEditor & docView;
public:
    DocListModel(TextEditor & docView);
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
    TextEditor docView;
    ListBox docList;
public:
    DocView();
    void setDocs(const map<string, ModuleDoc> & moduleDocs);
    void resized() override;
};
