#include "JuceLibraryCode/JuceHeader.h"
#include "PhasePhckrGrid.h"

void PhasePhckrGrid::paint(Graphics& g){
  int numberOfRows = gridComponents.size()/coloumnSizes.size();
  float size_y = (float)this->getHeight();
  float size_x = (float)this->getWidth();

  g.setColour(Colours::darkgrey);
  
  for(int i=0; i<numberOfRows; ++i){
    g.drawHorizontalLine(((float)i/(float)numberOfRows) * size_y, 0.0f, size_x);
  }
  g.drawHorizontalLine(size_y, 0.0f, size_x);

  g.drawVerticalLine(0, 0.0f, size_y);
  for(int i=0; i<coloumnSizes.size(); ++i){
    g.drawVerticalLine(coloumnSizes[i] * size_x, 0.0f, size_y);
  }

}

void PhasePhckrGrid::resized(){
  int numberOfRows = gridComponents.size()/coloumnSizes.size();

  int i=0;
  float x = 0;
  for(auto &c : gridComponents){
    //indices
    if ((i % coloumnSizes.size()) == 0) { x = 0; }
    float w = coloumnSizes[i%coloumnSizes.size()];
    float y = i/coloumnSizes.size();

    // set positions/size
    c->setBoundsRelative(
      x + 0.001f,// x
      y/(float)numberOfRows + 0.001f,       // y
      x + w - 0.002f,            // w
      1.f/(float)numberOfRows - 0.002f      // h
    );
    x += w;
    i++;
  }

  repaint();
}

void PhasePhckrGrid::addComponent(Component* component){
  addAndMakeVisible(*component);
  gridComponents.push_back(component);
  resized();
}

void PhasePhckrGrid::setColoumns(const std::vector<float> &sizes) {
    coloumnSizes = sizes;
    // normalize
    float sum = 0;
    for (const auto &s : coloumnSizes) {
        sum += s;
    }
    for (auto &s : coloumnSizes) {
        s /= sum;
    }
    resized();
}