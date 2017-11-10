#include "Style.hpp"

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
