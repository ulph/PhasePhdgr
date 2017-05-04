#include "GraphEditor.hpp"
#include "components.hpp"

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


ConnectionGraphTextEditor::ConnectionGraphTextEditor(SubValue<PhasePhckr::ConnectionGraphDescriptor> & sub)
    : TextEditor()
    , sub(sub)
{
    setMultiLine(true, true);
    handle = sub.subscribe(
        [this](const PhasePhckr::ConnectionGraphDescriptor g) {
        setText(json(g).dump(2));
    }
    );
    _stylize(this);
}
ConnectionGraphTextEditor::~ConnectionGraphTextEditor() {
    sub.unsubscribe(handle);
}


GraphViewBundle::GraphViewBundle(
    GraphEditor& graphEditor,
    const Doc& doc,
    SubValue<ConnectionGraphDescriptor> & subscribedCGD,
    const ModuleVariable& inBus,
    const ModuleVariable& outBus
)
    : graphView(
        graphEditor
        , viewPort
        , doc
        , subscribedCGD
        , inBus
        , outBus
    )
{
    addAndMakeVisible(viewPort);
    viewPort.setViewedComponent(&graphView, false);
    resized();
}
void GraphViewBundle::paint(Graphics& g)
{
    g.fillAll(Colours::black);
}
void GraphViewBundle::resized()
{
    viewPort.setBoundsRelative(0, 0, 1, 1);
    repaint();
}


GraphEditor::GraphEditor(
    const Doc &doc,
    SubValue<PhasePhckr::ConnectionGraphDescriptor> &subscribedCGD,
    const ModuleVariable &inBus,
    const ModuleVariable &outBus
)
    : doc(doc)
    , docListModel(doc.get(), docView)
    , docList("docList", &docListModel)
    , inBus(inBus)
    , outBus(outBus)
    , textEditor(subscribedCGD)
    , rootView(
        *this,
        doc,
        subscribedCGD,
        inBus,
        outBus
    )
    , editorStack(TabbedButtonBar::TabsAtTop)
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

    resized();
}

void GraphEditor::paint(Graphics& g)
{
    g.fillAll(Colours::black);
}

void GraphEditor::resized()
{
    grid.setBoundsRelative(0, 0, 1, 1);
    repaint();
}

void GraphEditor::push_tab(const string& componentName, const string& componentType) {
    auto docCopy = doc;
    const auto& d = docCopy.get();
    auto dit = d.find(componentType);
    if (dit != d.end()) {
        ComponentDescriptor cmp;
        if (componentRegister.getComponent(componentType, cmp)) {
            assert(componentType == dit->second.type);
            componentGraphs.push_back(SubValue<PhasePhckr::ConnectionGraphDescriptor>());
            auto &cdg = componentGraphs.back();

            string inBusType = "_"+componentType+"_INPUT";
            string outBusType = "_"+componentType+"_OUTPUT";

            ModuleDoc inDoc = d.at(componentType);
            inDoc.type = inBusType;
            inDoc.outputs = inDoc.inputs;
            inDoc.inputs.clear();
            docCopy.add(inDoc);

            ModuleDoc outDoc = d.at(componentType);
            outDoc.type = outBusType;
            outDoc.inputs = outDoc.outputs;
            outDoc.outputs.clear();
            docCopy.add(outDoc);

            ModuleVariable inBus = {"inBus", inBusType};
            ModuleVariable outBus = {"outBus", outBusType};

            for (const auto& ai : cmp.inputs) {
                cmp.graph.connections.push_back({ ModulePort{"inBus", ai.alias}, ai.wrapped });
            }

            for (const auto& ao : cmp.outputs) {
                cmp.graph.connections.push_back({ ao.wrapped, ModulePort{ "outBus", ao.alias } });
            }

            editorStack.addTab(
                to_string(componentGraphs.size()) + " " + componentName + " (" + componentType + ") ",
                Colours::black,
                new GraphViewBundle(
                    *this,
                    docCopy,
                    cdg,
                    inBus,
                    outBus
                ),
                true
            );

            cdg.set(-1, cmp.graph);

        }
    }
}