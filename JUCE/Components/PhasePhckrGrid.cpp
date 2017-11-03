#include "JuceHeader.h"
#include "PhasePhckrGrid.h"

void PhasePhckrGrid::paint(Graphics& g){
  float numberOfRows = (float)gridComponents.size()/coloumnSizes.size();
  float size_y = (float)this->getHeight();
  float size_x = (float)this->getWidth();

  g.setColour(Colour(0xff111111));
  for(float i=0; i<numberOfRows; ++i){
    g.drawHorizontalLine((int)((i/numberOfRows) * size_y), 0.0f, size_x);
  }
  g.drawHorizontalLine((int)size_y, 0.0f, size_x);

  int x = 0;
  g.drawVerticalLine(x, 0.0f, size_y);
  for(int i=0; i<coloumnSizes.size(); ++i){
    x += (int)(coloumnSizes[i] * size_x);
    g.drawVerticalLine(x, 0.0f, size_y);
  }
}

void PhasePhckrGrid::resized(){
  float numberOfRows = (float)(gridComponents.size()/coloumnSizes.size());

  int i=0;
  float x = 0;
  for(auto &c : gridComponents){
    //indices
    if ((i % coloumnSizes.size()) == 0) { x = 0; }
    float w = coloumnSizes[i%coloumnSizes.size()];
    float y = (float)(i/coloumnSizes.size());

    // set positions/size
    c->setBoundsRelative(
      x,
      y/numberOfRows,
      w,
      1.f/numberOfRows
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