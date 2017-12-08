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
public:
    GraphEditorTabbedComponent(vector<string> & subPatchTypes)
        : TabbedComponent(TabbedButtonBar::TabsAtTop)
        , subPatchTypes(subPatchTypes)
    {
    }
    virtual void currentTabChanged(int newCurrentTabIndex, const String& newCurrentTabName) {
        while (newCurrentTabIndex + 1 != getNumTabs()) {
            removeTab(getNumTabs() - 1);
            subPatchTypes.pop_back();
        }
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

    set<string> globalComponents; // todo, pass along into all the editors
    PhasePhckr::ComponentRegister cmpReg;
    SubValue<PhasePhckr::ComponentRegister> &subCmpReg;
    int cmpRegHandle;

    PhasePhckrGrid grid;

    GraphEditorBundle rootView;

    vector<string> subPatchTypes;

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
