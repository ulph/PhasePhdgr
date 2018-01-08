#include "JuceHeader.h"
#include "PPGrid.h"

void PPGrid::paint(Graphics& g){
  float numberOfRows = ceilf(((float)components.size()) / ((float)coloumnSizes.size()));

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

void PPGrid::resized(){
  float numberOfRows = ceilf(((float)components.size()) / ((float)coloumnSizes.size()));

  int i=0;
  float x = 0;
  for(auto &c : components){
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

void PPGrid::addComponent(Component* component){
  addAndMakeVisible(*component);
  components.push_back(component);
  resized();
}

void PPGrid::setColoumns(const std::vector<float> &sizes) {
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

PPGGrid::PPGGrid() {
    addAndMakeVisible(grid);
    resized();
}

void PPGGrid::addComponent(Component* component) {
    grid.addComponent(component);
}

void PPGGrid::setColoumns(const vector<float> &coloumnSizes) {
    grid.setColoumns(coloumnSizes);
}

void PPGGrid::resized() {
    grid.setBoundsRelative(0.1f, 0.1f, 0.8f, 0.8f);
    grid.resized();
}