#include <phasephckr.hpp>
#include <phasephckr_json.hpp>

#include "PatchEditor.hpp"

using namespace std;
using namespace PhasePhckr;


void populateDocWithComponents(Doc & doc, const PhasePhckr::ComponentRegister cr, const PatchDescriptor pd){
    cr.makeComponentDocs(doc);
    for (const auto & c : pd.components) {
        ModuleDoc d;
        PhasePhckr::ComponentRegister::makeComponentDoc(c.first, c.second, d);
        doc.add(d);
    }
}


void PatchEditor::refreshAndBroadcastDoc(){
    doc = Doc();
    populateDocWithComponents(doc, cmpReg, patchCopy);

    set<string> localComponents;
    for (const auto& kv : patchCopy.components) localComponents.insert(kv.first);

    docView.setLocalComponents(localComponents);
    docView.setGlobalComponents(globalComponents);
    docView.setDocs(doc.get());

    rootBundle.editor.setDoc(doc);
    rootBundle.editor.setGlobalComponents(globalComponents);

    for (auto* b : subPatchBundles) {
        b->editor.setGlobalComponents(globalComponents);
        b->editor.setDoc(doc);
    }

    repaint();
}


PatchEditor::PatchEditor(
    SubValue<PatchDescriptor> &subPatch,
    SubValue<PhasePhckr::ComponentRegister> &subCmpReg,
    const vector<PadDescription> &inBus,
    const vector<PadDescription> &outBus
)
    : subPatch(subPatch)
    , subCmpReg(subCmpReg)
    , rootBundle(
       *this,
       doc,
       subPatch,
       "root",
       patchCopy,
       inBus,
       outBus
    )
    , editorStack(subPatchTypes, subPatchBundles)
{
    addAndMakeVisible(grid);
    grid.addComponent(&editorStack);
    grid.addComponent(&docView);
    grid.setColoumns({ 0.875f, 0.125f });

    editorStack.addTab("root", Colours::black, &rootBundle, false);

    patchHandle = subPatch.subscribe(
        [this](const PatchDescriptor& desc) {
            patchCopy = desc;
            refreshAndBroadcastDoc();
            for (int i = 0; i < subPatchTypes.size(); ++i) {
                auto t = subPatchTypes.at(i);
                if (!desc.components.count(t)) editorStack.setCurrentTabIndex(i); // +1 (root) -1 (previous)
            }
        }
    );

    cmpRegHandle = subCmpReg.subscribe(
        [this](const PhasePhckr::ComponentRegister& cmpReg_){
            cmpReg = cmpReg_;
            applyComponentRegister();
        }
    );

    refreshAndBroadcastDoc();

    resized();
}

PatchEditor::~PatchEditor() {
    subPatch.unsubscribe(patchHandle);
    subCmpReg.unsubscribe(cmpRegHandle);
}

void PatchEditor::applyComponentRegister() {
    globalComponents.clear();
    for (const auto& kv : cmpReg.all()) {
        globalComponents.insert(kv.first);
    }
    refreshAndBroadcastDoc();
}

void PatchEditor::paint(Graphics& g)
{
    g.fillAll(Colours::darkgrey);
}

void PatchEditor::resized()
{
    grid.setBoundsRelative(0, 0, 1, 1);
    repaint();
}

void PatchEditor::push_tab(const string& componentName, const string& componentType) {
    ComponentDescriptor cmp;

    if (!patchCopy.components.count(componentType)) {
        if(!cmpReg.getComponent(componentType, cmp)) return;
        patchCopy.components[componentType] = cmp;
    }

    cmp = patchCopy.components.at(componentType);
    subPatchTypes.push_back(componentType);
    subPatchBundles.push_back(
        new GraphEditorBundle(
            *this,
            doc,
            subPatch,
            componentType,
            patchCopy,
            cmp.inBus,
            cmp.outBus
        )
    );

    auto* b = subPatchBundles.back();

    PatchDescriptor p;
    p.root = cmp;

    editorStack.addTab(
        to_string(subPatchTypes.size()) + " " + componentName + " (" + componentType + ") ",
        Colours::black,
        b,
        true
    );

    editorStack.setCurrentTabIndex(editorStack.getNumTabs()-1);

}
