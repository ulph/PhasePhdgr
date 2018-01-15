#pragma once

#include <map>
#include <vector>

#include <phasephckr.hpp>
#include "JuceHeader.h"

#include "PPGrid.h"
#include "Utils.hpp"
#include "GraphEditor.hpp"

#include "DocView.hpp"

using namespace PhasePhckr;
using namespace std;

const string rootMarker = "root";

const struct ComponentMenuStrings {
    const string createInput = "create input";
    const string createOutput = "create output";
    const string removeConflict = "remove conflicting Component definition";
    const string removeLocal = "(!) remove local Component definition";
    const string createLocal = "create local Component definition";
    const string clone = "change into new Component";
    const string docString = "change docstring";
} c_componentMenuStrings;

struct ComponentPopupMenuState {
    int typeMenuId = 999;
    TextLabelMenuEntry name;
    int createInputMenuId = 999;
    int createOutputMenuId = 999;
    int removeConflictingComponentMenuId = 999;
    int removeLocalComponentMenuId = 999;
    int addLocalComponentMenuId = 999;
    int docStringMenuId = 999;
    TextEditor docStringEditor;
};

void makeComponentPopupMenu(
    PopupMenu & poop,
    int& ctr,
    ComponentPopupMenuState& ids,
    const string& type,
    const PatchDescriptor& patch,
    const map<string, ComponentDescriptor>& global,
    const map<string, ComponentDescriptor>& local
);

bool applyComponentPopuMenuChoice(
    int choice,
    const ComponentPopupMenuState& ids,
    const string& type,
    PatchDescriptor& patch,
    const map<string, ComponentDescriptor>& global
);

class GraphEditorTabbedComponent : public TabbedComponent {
private:
    vector<string> & subPatchTypes;
    vector<GraphEditorBundle*> & subPatchBundles;
public:
    GraphEditorTabbedComponent(vector<string> & subPatchTypes, vector<GraphEditorBundle*> & subPatchBundles)
        : TabbedComponent(TabbedButtonBar::TabsAtTop)
        , subPatchTypes(subPatchTypes)
        , subPatchBundles(subPatchBundles)
    {
    }
    virtual void currentTabChanged(int newCurrentTabIndex, const String& newCurrentTabName) {
        while (newCurrentTabIndex + 1 != getNumTabs()) {
            subPatchTypes.pop_back();
            subPatchBundles.pop_back();
            removeTab(getNumTabs() - 1);
        }
    }
};

class PatchEditor : public Component
{
    Doc doc;
    DocView docView;

    SubValue<PatchDescriptor> &subPatch;
    PatchDescriptor patchCopy;
    int patchHandle;

    set<string> globalComponents;
    PhasePhckr::ComponentRegister cmpReg;
    SubValue<PhasePhckr::ComponentRegister> &subCmpReg;
    int cmpRegHandle;

    PPGrid PPGrid;

    GraphEditorBundle rootBundle;

    vector<string> subPatchTypes;
    vector<GraphEditorBundle*> subPatchBundles;

    LayoutUpdateCallback layoutUpdateCb;

    GraphEditorTabbedComponent editorStack;
    void refreshAndBroadcastDoc();
    friend class GraphEditor;
    void push_tab(const string& componentName, const string& componentType);

    void applyComponentRegister();

public:
    PatchEditor(
        SubValue<PatchDescriptor> &subPatch,
        SubValue<PhasePhckr::ComponentRegister> &subCmpReg,
        const vector<PadDescription> &inBus,
        const vector<PadDescription> &outBus,
        LayoutUpdateCallback layoutUpdateCb
    );
    virtual ~PatchEditor();
    void paint(Graphics& g);
    void resized();

};
