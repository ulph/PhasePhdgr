#pragma once

#include <map>
#include <vector>

#include <phasephckr.hpp>
#include "JuceHeader.h"

#include "PhasePhckrGrid.h"
#include "Utils.hpp"
#include "GraphEditor.hpp"

using namespace PhasePhckr;
using namespace std;

void _stylize(Label * l);
void _stylize(TextEditor* t);
void _stylize(ListBox* l);
void _stylize(FileListComponent* l);

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
    virtual var getDragSourceDescription (const SparseSet< int > &rowsToDescribe){
        if(!rowsToDescribe.isEmpty()){
            return String(rows[rowsToDescribe[0]]);
        }
        return var();
    }
};


class GraphEditorTabbedComponent : public TabbedComponent {
private:
    list<SubValue<PatchDescriptor>> & subPatches;
    list<int> & subPatchHandles;
public:
    GraphEditorTabbedComponent(list<SubValue<PatchDescriptor>> & subPatches, list<int> & subPatchHandles)
        : TabbedComponent(TabbedButtonBar::TabsAtTop)
        , subPatches(subPatches)
        , subPatchHandles(subPatchHandles)
    {
        assert(subPatches.size() == subPatchHandles.size());
    }
    virtual void currentTabChanged(int newCurrentTabIndex, const String& newCurrentTabName) {
        while (newCurrentTabIndex + 1 != getNumTabs()) {
            removeTab(getNumTabs() - 1);
            auto h = subPatchHandles.back(); subPatchHandles.pop_back();
            subPatches.back().unsubscribe(h); subPatches.pop_back();
        }
        assert(subPatches.size() == subPatchHandles.size());
    }
};


class PatchEditor : public Component
{
    Doc doc;
    SubValue<Doc> subDoc;
    int docHandle;

    SubValue<PatchDescriptor> &subPatch;
    PatchDescriptor patchCopy;
    int patchHandle;

    PhasePhckr::ComponentRegister cmpReg;
    SubValue<PhasePhckr::ComponentRegister> &subCmpReg;
    int cmpRegHandle;

    PhasePhckrGrid grid;
    PhasePhckrGrid docGrid;

    GraphEditorBundle rootView;

    DocListModel docListModel;
    TextEditor docView;
    ListBox docList;

    list<SubValue<PatchDescriptor>> subPatches;
    list<int> subPatchHandles;
    GraphEditorTabbedComponent editorStack;
    void refreshAndBroadcastDoc();
    friend class GraphEditor;
    void push_tab(const string& componentName, const string& componentType);

public:
    PatchEditor(
        SubValue<PatchDescriptor> &subPatch,
        SubValue<PhasePhckr::ComponentRegister> &subCmpReg,
        const vector<PadDescription> &inBus,
        const vector<PadDescription> &outBus
    );
    virtual ~PatchEditor();
    void paint(Graphics& g);
    void resized();

};
