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
    DocView docView;

    SubValue<PatchDescriptor> &subPatch;
    PatchDescriptor patchCopy;
    int patchHandle;

    PhasePhckr::ComponentRegister cmpReg;
    SubValue<PhasePhckr::ComponentRegister> &subCmpReg;
    int cmpRegHandle;

    PhasePhckrGrid grid;

    GraphEditorBundle rootView;

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
