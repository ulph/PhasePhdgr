#pragma once

#include <map>
#include <vector>

#include <phasephckr.hpp>
#include "JuceHeader.h"

#include "PhasePhckrGrid.h"
#include "Utils.hpp"
#include "GraphEditor.hpp"

#include "DocView.hpp"

using namespace PhasePhckr;
using namespace std;

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

    set<string> globalComponents; // todo, pass along into all the editors
    PhasePhckr::ComponentRegister cmpReg;
    SubValue<PhasePhckr::ComponentRegister> &subCmpReg;
    int cmpRegHandle;

    PhasePhckrGrid grid;

    GraphEditorBundle rootBundle;

    vector<string> subPatchTypes;
    vector<GraphEditorBundle*> subPatchBundles;

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
