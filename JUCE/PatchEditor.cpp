#include "PatchEditor.hpp"
#include "components.hpp"

void _stylize(Label * l) {
    l->setJustificationType(Justification::centred);
    l->setColour(Label::backgroundColourId, Colours::darkgrey);
    l->setColour(Label::backgroundWhenEditingColourId, Colours::white);
    l->setColour(Label::textColourId, Colours::white);
    l->setColour(Label::textWhenEditingColourId, Colours::black);
    l->setColour(Label::outlineColourId, Colours::black);
    l->setColour(Label::outlineWhenEditingColourId, Colours::black);
}

void _stylize(TextEditor* t) {
    t->setMultiLine(true, true);
    t->setColour(TextEditor::backgroundColourId, Colours::black);
    t->setColour(TextEditor::textColourId, Colours::green);
    t->setColour(TextEditor::highlightColourId, Colours::darkgreen);
    t->setColour(TextEditor::highlightedTextColourId, Colours::yellow);
    t->setColour(TextEditor::outlineColourId, Colours::black);
    t->setColour(TextEditor::focusedOutlineColourId, Colours::black);
    t->setColour(TextEditor::shadowColourId, Colours::black);
}

void _stylize(ListBox* l) {
    l->setMultipleSelectionEnabled(false);
    l->setColour(ListBox::backgroundColourId, Colours::black);
    l->setColour(ListBox::textColourId, Colours::green);
    l->setColour(ListBox::outlineColourId, Colours::black);
}

void _stylize(FileListComponent* l) {
    l->setMultipleSelectionEnabled(false);
    l->setColour(FileListComponent::backgroundColourId, Colours::black);
    l->setColour(FileListComponent::outlineColourId, Colours::black);
    l->setColour(FileListComponent::highlightColourId, Colours::darkgreen);
    l->setColour(DirectoryContentsDisplayComponent::textColourId, Colours::green);
    l->setColour(DirectoryContentsDisplayComponent::highlightColourId, Colours::darkgreen);
}

DocListModel::DocListModel(const std::map<std::string, ModuleDoc> & moduleDocs, TextEditor & docView)
    : ListBoxModel()
    , moduleDocs(moduleDocs)
    , docView(docView)
{
    for (const auto &kv : moduleDocs) {
        rows.push_back(kv.first);
    }
}

void DocListModel::setDocs(const std::map<std::string, ModuleDoc> & moduleDocs_) {
    moduleDocs = moduleDocs_;
    rows.clear();
    for (const auto &kv : moduleDocs) {
        rows.push_back(kv.first);
    }
}

int DocListModel::getNumRows() {
    return (int)rows.size();
}

void DocListModel::paintListBoxItem(int rowNumber, Graphics &g, int width, int height, bool rowIsSelected) {
    g.setColour(Colours::black);
    g.fillAll();
    g.setColour(Colours::green);
    const auto &key = rows[rowNumber];
    const auto &doc = moduleDocs.at(key);
    g.drawFittedText(doc.type, 0, 0, width, height, Justification::centred, 1);
}

void DocListModel::listBoxItemClicked(int row, const MouseEvent &) {
    if (row >= 0) {
        const auto &key = rows[row];
        const auto &doc = moduleDocs[key];
        docView.clear();
        docView.insertTextAtCaret(doc.type + "\n\n");
        docView.insertTextAtCaret("inputs:\n");
        for (const auto & i : doc.inputs) {
            docView.insertTextAtCaret("  " + i.name + ((i.unit != "") ? (" [" + i.unit + "]") : "") + " " + std::to_string(i.value) + "\n");
        }
        docView.insertTextAtCaret("\noutputs:\n");
        for (const auto & o : doc.outputs) {
            docView.insertTextAtCaret("  " + o.name + ((o.unit != "") ? (" [" + o.unit + "]") : "") + "\n");
        }
        docView.insertTextAtCaret("\n\n" + doc.docString);
        docView.moveCaretToTop(false);
    }
}


PatchTextEditor::PatchTextEditor(SubValue<PatchDescriptor> & sub)
    : TextEditor()
    , sub(sub)
{
    setMultiLine(true, true);
    handle = sub.subscribe(
        [this](const PhasePhckr::PatchDescriptor g) {
        setText(json(g).dump(2));
    }
    );
    _stylize(this);
}
PatchTextEditor::~PatchTextEditor() {
    sub.unsubscribe(handle);
}

PatchEditor::PatchEditor(
    const Doc &doc_,
    SubValue<PatchDescriptor> &subPatch,
    const vector<PadDescription> &inBus,
    const vector<PadDescription> &outBus
)
    : doc(doc_)
    , subPatch(subPatch)
    , docListModel(doc.get(), docView)
    , docList("docList", &docListModel)
    , textEditor(subPatch)
    , rootView(
        *this,
        doc,
        subPatch,
        inBus,
        outBus
    )
    , editorStack(subPatches, subPatchHandles)
{
    addAndMakeVisible(grid);
    grid.addComponent(&textEditor);
    grid.addComponent(&editorStack);
    grid.addComponent(&docGrid);
    grid.setColoumns({ 0.125f, 0.75f, 0.125f });

    editorStack.addTab("root", Colours::black, &rootView, false);

    docGrid.addComponent(&docView);
    docGrid.addComponent(&docList);
    docGrid.setColoumns({ 1.0f });
    docList.updateContent();

    _stylize(&docView);
    _stylize(&docList);

    patchHandle = subPatch.subscribe(
        function<void(const PatchDescriptor&)>([this](const PatchDescriptor& desc) {
            patchCopy = desc;
            PhasePhckr::ComponentRegister().makeComponentDocs(doc); // HAX 1
            for (const auto & c : desc.components) { // HAX 2
                ModuleDoc d;
                PhasePhckr::ComponentRegister::makeComponentDoc(c.first, c.second, d);
                doc.add(d);
            }
            docListModel.setDocs(doc.get());
            docList.repaint();
        }
    ));

    resized();
}

PatchEditor::~PatchEditor() {
    subPatch.unsubscribe(patchHandle);
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
        // HAX, the component register needs to be passed here or onto this
        PhasePhckr::ComponentRegister cr;
        if(!cr.getComponent(componentType, cmp)) return;
        patchCopy.components[componentType] = cmp;
    }

    cmp = patchCopy.components.at(componentType);
    subPatches.push_back(SubValue<PatchDescriptor>());
    auto &subP = subPatches.back();
    auto handle = subP.subscribe(function<void(const PatchDescriptor&)>(
        [this, cmp, componentType](const PatchDescriptor& p){
        patchCopy.components[componentType] = cmp;
        patchCopy.components[componentType].graph = p.root.graph;
        subPatch.set(-1, patchCopy);
    }
    ));
    subPatchHandles.push_back(handle);

    PatchDescriptor p;
    p.root = cmp;

    editorStack.addTab(
        to_string(subPatches.size()) + " " + componentName + " (" + componentType + ") ",
        Colours::black,
        new GraphEditorBundle(
            *this,
            doc,
            subP,
            cmp.inBus,
            cmp.outBus
        ),
        true
    );

    subP.set(handle, p);

    editorStack.setCurrentTabIndex(editorStack.getNumTabs()-1);

}
