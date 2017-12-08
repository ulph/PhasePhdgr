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
    docView.setDocs(doc.get());
    subDoc.set(docHandle, doc);
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
    , rootView(
       *this,
       subDoc,
       subPatch,
       inBus,
       outBus
    )
    , editorStack(subPatches, subPatchHandles, subPatchTypes)
{
    addAndMakeVisible(grid);
    grid.addComponent(&editorStack);
    grid.addComponent(&docView);
    grid.setColoumns({ 0.875f, 0.125f });

    editorStack.addTab("root", Colours::black, &rootView, false);

    patchHandle = subPatch.subscribe(
        [this](const PatchDescriptor& desc) {
            patchCopy = desc;
            refreshAndBroadcastDoc();
            for (int i = 0; i < subPatchTypes.size(); ++i) {
                auto t = subPatchTypes.at(i);
                if (desc.components.count(t)) {
                    PatchDescriptor p;
                    p.root = desc.components.at(t);
                    auto subP = subPatches.at(i);
                    auto h = subPatchHandles.at(i);
                    subP.set(h, p);
                }
                else {
                    editorStack.setCurrentTabIndex(i); // +1 (root) -1 (previous)
                }
            }
        }
    );

    docHandle = subDoc.subscribe(
        [this](const Doc& doc_){
            assert(0); // nobody else should be able to do this ... TODO, better solution :P
        }
    );

    cmpRegHandle = subCmpReg.subscribe(
        [this](const PhasePhckr::ComponentRegister& cmpReg_){
            cmpReg = cmpReg_;
            refreshAndBroadcastDoc();
        }
    );

    resized();
}

PatchEditor::~PatchEditor() {
    subPatch.unsubscribe(patchHandle);
    subCmpReg.unsubscribe(cmpRegHandle);
    subDoc.unsubscribe(docHandle);
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
    subPatches.push_back(SubValue<PatchDescriptor>());
    auto &subP = subPatches.back();
    auto handle = subP.subscribe(
        [this, cmp, componentType](const PatchDescriptor& p){
            patchCopy.components[componentType] = cmp;
            patchCopy.components[componentType].graph = p.root.graph;
            subPatch.set(-1, patchCopy);
        }
    );
    subPatchHandles.push_back(handle);
    subPatchTypes.push_back(componentType);

    PatchDescriptor p;
    p.root = cmp;

    editorStack.addTab(
        to_string(subPatches.size()) + " " + componentName + " (" + componentType + ") ",
        Colours::black,
        new GraphEditorBundle(
            *this,
            subDoc,
            subP,
            cmp.inBus,
            cmp.outBus
        ),
        true
    );

    subP.set(handle, p);
    subDoc.set(docHandle, doc);

    editorStack.setCurrentTabIndex(editorStack.getNumTabs()-1);

}
