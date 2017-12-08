#include <phasephckr.hpp>
#include <phasephckr_json.hpp>

#include "PatchEditor.hpp"

using namespace std;
using namespace PhasePhckr;

void makeComponentPopupMenu(PopupMenu & poop,
    int& ctr,
    ComponentPopupMenuState& ids,
    const string& type,
    const PatchDescriptor& patch,
    const map<string, ComponentDescriptor>& global,
    const map<string, ComponentDescriptor>& local)
{
    ids.name.title.setText("Type:", NotificationType::dontSendNotification);
    ids.name.edit.setText(type, NotificationType::dontSendNotification);
    ids.name.edit.setEditable(true, true, false);

    if (local.count(type)) {
        ids.typeMenuId = ctr++;
        poop.addCustomItem(ids.typeMenuId, &ids.name, 200, 20, false);

        ids.createInputMenuId = ctr++;
        poop.addItem(ids.createInputMenuId, c_componentMenuStrings.createInput);

        ids.createOutputMenuId = ctr++;
        poop.addItem(ids.createOutputMenuId, c_componentMenuStrings.createOutput);
    }

    if (global.count(type) && local.count(type)) {
        ids.removeLocalComponentMenuId = ctr++;
        poop.addItem(ids.removeLocalComponentMenuId, c_componentMenuStrings.removeConflict);
    }
    else if (!global.count(type) && local.count(type)) {
        ids.removeConflictingComponentMenuId = ctr++;
        poop.addItem(ids.removeConflictingComponentMenuId, c_componentMenuStrings.removeLocal);
    }
    else if (global.count(type) && !local.count(type)) {
        ids.addLocalComponentMenuId = ctr++;
        poop.addItem(ids.addLocalComponentMenuId, c_componentMenuStrings.createLocal);
    }
}

bool applyComponentPopuMenuChoice(
    int choice,
    const ComponentPopupMenuState& ids,
    const string& type,
    PatchDescriptor& patch,
    const map<string, ComponentDescriptor>& global
){
    if (choice == ids.createInputMenuId || choice == ids.createOutputMenuId) {
        if (!patch.components.count(type)) return false;
        auto & comp = patch.components.at(type);
        return 0 == patch.components.at(type).addPort("newPort", choice == ids.createInputMenuId, "", 0.0f);
    }
    else if (choice == ids.removeConflictingComponentMenuId) {
        return 0 == patch.removeComponentType(type, true);
    }
    else if (choice == ids.removeLocalComponentMenuId) {
        return 0 == patch.removeComponentType(type, false);
    }
    else if (choice == ids.addLocalComponentMenuId) {
        if (!global.count(type)) return false;
        string type_ = type;
        return 0 == patch.addComponentType(type_, global.at(type));
    }
    else if (type != ids.name.edit.getText().toStdString()) {
        return 0 == patch.renameComponentType(type, ids.name.edit.getText().toStdString());
    }

    return false;
}

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
    rootBundle.editor.setGlobalComponents(cmpReg.all());

    for (auto* b : subPatchBundles) {
        b->editor.setGlobalComponents(cmpReg.all());
        b->editor.setDoc(doc);
    }

    repaint();
}


PatchEditor::PatchEditor(
    SubValue<PatchDescriptor> &subPatch_,
    SubValue<PhasePhckr::ComponentRegister> &subCmpReg,
    const vector<PadDescription> &inBus,
    const vector<PadDescription> &outBus
)
    : subPatch(subPatch_)
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
    , editorStack(
        subPatchTypes, 
        subPatchBundles
    )
    , docView(
        [this](const string& name, const MouseEvent& me) {
            if (me.mods.isRightButtonDown()) {
                if (patchCopy.components.count(name) || globalComponents.count(name)){
                    int ctr = 1;
                    PopupMenu poop;
                    ComponentPopupMenuState st;
                    makeComponentPopupMenu(poop, ctr, st, name, patchCopy, cmpReg.all(), patchCopy.components);
                    auto choice = poop.show();
                    if (applyComponentPopuMenuChoice(choice, st, name, patchCopy, cmpReg.all())) {
                        subPatch.set(-1, patchCopy);
                    }
                }
            }
        }
    )
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
