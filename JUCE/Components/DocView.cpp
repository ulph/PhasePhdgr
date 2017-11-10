#include "DocView.hpp"
#include "Style.hpp"

DocListModel::DocListModel(TextEditor & docView)
    : ListBoxModel()
    , docView(docView)
{
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

var DocListModel::getDragSourceDescription (const SparseSet< int > &rowsToDescribe){
    if(!rowsToDescribe.isEmpty()){
        return String(rows[rowsToDescribe[0]]);
    }
    return var();
}


DocView::DocView()
    : docListModel(docView)
    , docList("docList", &docListModel)
{
    addAndMakeVisible(docGrid);

    docGrid.addComponent(&docView);
    docGrid.addComponent(&docList);
    docGrid.setColoumns({ 1.0f });
    docList.updateContent();

    _stylize(&docView);
    _stylize(&docList);

    resized();
}

void DocView::setDocs(const std::map<std::string, ModuleDoc> & moduleDocs_) {
    docListModel.setDocs(moduleDocs_);
    docList.updateContent();
}

void DocView::resized(){
    docGrid.setBoundsRelative(0.f, 0.f, 1.f, 1.f);
}
