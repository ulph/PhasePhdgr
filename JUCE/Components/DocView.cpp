#include "DocView.hpp"

DocListModel::DocListModel(TextEditor & docTextView, const ComponentClickedCallback& componentClicked)
    : ListBoxModel()
    , docTextView(docTextView)
    , componentClicked(componentClicked)
{
}

void DocListModel::setDocs(const std::map<std::string, ModuleDoc> & moduleDocs_) {
    moduleDocs = moduleDocs_;
    rows.clear();
    for (const auto &kv : moduleDocs) {
        rows.push_back(kv.first);
    }
    drawEntry(lastKey);
}

void DocListModel::setGlobalComponents(const set<string>& globalComponents_) {
    globalComponents = globalComponents_;
}

void DocListModel::setLocalComponents(const set<string>& localComponents_) {
    localComponents = localComponents_;
}

int DocListModel::getNumRows() {
    return (int)rows.size();
}

void DocListModel::paintListBoxItem(int rowNumber, Graphics &g, int width, int height, bool rowIsSelected) {
    const auto &key = rows[rowNumber];
    const auto &doc = moduleDocs.at(key);

    g.setColour(Colours::black);
    g.fillAll();
    g.setColour(Colours::green);

    if (localComponents.count(doc.type) && !globalComponents.count(doc.type)) 
        g.setColour(Colours::yellow);
    else if (localComponents.count(doc.type) && globalComponents.count(doc.type)) 
        g.setColour(Colours::red);

    g.drawFittedText(doc.type, 0, 0, width, height, Justification::centred, 1);
}

void DocListModel::drawEntry(const string& key) {
    docTextView.clear();
    if (!moduleDocs.count(key)) return;
    const auto &doc = moduleDocs.at(key);
    docTextView.insertTextAtCaret(doc.type + "\n\n");
    docTextView.insertTextAtCaret("inputs:\n");
    for (const auto & i : doc.inputs) {
        docTextView.insertTextAtCaret("  " + i.name + ((i.unit != "") ? (" [" + i.unit + "]") : "") + " " + std::to_string(i.defaultValue) + "\n");
    }
    docTextView.insertTextAtCaret("\noutputs:\n");
    for (const auto & o : doc.outputs) {
        docTextView.insertTextAtCaret("  " + o.name + ((o.unit != "") ? (" [" + o.unit + "]") : "") + "\n");
    }
    docTextView.insertTextAtCaret("\n\n" + doc.docString);
    docTextView.moveCaretToTop(false);
}

void DocListModel::listBoxItemClicked(int row, const MouseEvent &me) {
    if (row >= 0) {
        const auto &key = rows[row];
        lastKey = key;
        componentClicked(key, me); // trigger callback so external guy can use
        drawEntry(key);
    }
}

var DocListModel::getDragSourceDescription (const SparseSet< int > &rowsToDescribe){
    if(!rowsToDescribe.isEmpty()){
        return String(rows[rowsToDescribe[0]]);
    }
    return var();
}


DocView::DocView(const ComponentClickedCallback& cb)
    : docListModel(docTextView, cb)
    , docList("docList", &docListModel)
{
    addAndMakeVisible(docPPGrid);

    docPPGrid.addComponent(&docTextView);
    docPPGrid.addComponent(&docList);
    docPPGrid.setColoumns({ 1.0f });
    docList.updateContent();

    docTextView.setMultiLine(true);
    docTextView.setReadOnly(true);

    resized();
}

void DocView::setDocs(const std::map<std::string, ModuleDoc> & moduleDocs_) {
    docListModel.setDocs(moduleDocs_);
    docList.updateContent();
}

void DocView::setGlobalComponents(const set<string>& globalComponents) {
    docListModel.setGlobalComponents(globalComponents);
    docList.updateContent();
}

void DocView::setLocalComponents(const set<string>& localComponents) {
    docListModel.setLocalComponents(localComponents);
    docList.updateContent();
}

void DocView::resized(){
    docPPGrid.setBoundsRelative(0.f, 0.f, 1.f, 1.f);
}
