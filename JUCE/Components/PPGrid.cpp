#include "PPGrid.h"

void PPGrid::resized(){
  float numberOfRows = ceilf(((float)components.size()) / ((float)coloumnSizes.size()));

  int padding = 2;

  int i=0;
  float x = 0;
  for(auto &c : components){
    //indices
    if ((i % coloumnSizes.size()) == 0) { x = 0; }
    float w = coloumnSizes[i%coloumnSizes.size()];
    float y = (float)(i/coloumnSizes.size());

    // set positions/size
    c->setBounds(
        (int)(x * getWidth()) + padding,
        (int)(y / numberOfRows * getHeight()) + padding,
        (int)(w * getWidth()) - 2 * padding,
        (int)(1.f / numberOfRows * getHeight()) - 2 * padding
    );

    x += w;
    i++;
  }
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
    int padding = 30;
    grid.setBounds(padding, padding, getWidth() - 2*padding, getHeight() - 2*padding);
    grid.resized();
}