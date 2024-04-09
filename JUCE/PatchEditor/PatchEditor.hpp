#pragma once

#include <map>
#include <vector>

#include <phasephdgr.hpp>

#include "PPGrid.h"
#include "Utils.hpp"
#include "GraphEditor.hpp"

#include "DocView.hpp"

using namespace PhasePhdgr;
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
    std::function<Doc()> docFactory;
    Doc doc;
    DocView docView;
    ModuleDoc inBusDoc;
    ModuleDoc outBusDoc;

    SubValue<PatchDescriptor> &subPatch;
    PatchDescriptor patchCopy;

    int patchHandle;

    set<string> globalComponents;
    PhasePhdgr::ComponentRegister cmpReg;
    SubValue<PhasePhdgr::ComponentRegister> &subCmpReg;
    int cmpRegHandle;

    PPGrid ppgrid;

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
        std::function<Doc()> docFactory,
        SubValue<PatchDescriptor> &subPatch,
        SubValue<PhasePhdgr::ComponentRegister> &subCmpReg,
        const vector<PadDescription> &inBus,
        const vector<PadDescription> &outBus,
        LayoutUpdateCallback layoutUpdateCb
    );
    virtual ~PatchEditor();
    void paint(Graphics& g);
    void resized();
    void showDoc(const string& type);

};
