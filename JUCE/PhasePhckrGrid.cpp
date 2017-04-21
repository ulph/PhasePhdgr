#include "../JuceLibraryCode/JuceHeader.h"
#include "PhasePhckrGrid.h"

void PhasePhckrGrid::paint(Graphics& g){
  int numberOfRows = gridComponents.size()/numberOfColumns;
  float size_y = (float)this->getHeight();
  float size_x = (float)this->getWidth();

  g.setColour(Colours::grey);
  
  for(int i=0; i<numberOfRows; ++i){
    g.drawHorizontalLine(((float)i/(float)numberOfRows) * size_y, 0.0f, size_x);
  }

  for(int i=0; i<numberOfColumns; ++i){
    g.drawVerticalLine(((float)i/(float)numberOfColumns) * size_x, 0.0f, size_y);
  }

  g.drawVerticalLine(size_x, 0.0f, size_y);
  g.drawHorizontalLine(size_y, 0.0f, size_x);
}

void PhasePhckrGrid::resized(){
  if(numberOfColumns<1) return; // bail out
  int numberOfRows = gridComponents.size()/numberOfColumns;

  int i=0;
  for(auto &c : gridComponents){
    //indices
    float x = i%numberOfColumns;
    float y = i/numberOfColumns;

    // set positions/size
    c->setBoundsRelative(
      x/(float)numberOfColumns + 0.001f,   // x
      y/(float)numberOfRows + 0.001f,      // y
      1.f/(float)numberOfColumns - 0.002f, // w
      1.f/(float)numberOfRows - 0.002f    // h
    );
    i++;
  }

  repaint();
}

void PhasePhckrGrid::addComponent(Component* component){
  addAndMakeVisible(*component);
  gridComponents.push_back(component);
  resized();
}

void PhasePhckrGrid::setNumberOfColumns(int n) {
    if (n > 0) {
        numberOfColumns = n;
        resized();
    }
}