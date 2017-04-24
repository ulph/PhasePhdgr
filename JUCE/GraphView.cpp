#include "GraphView.h"
#include <algorithm>

typedef std::map<std::string, std::set<std::string>> ConnectionsMap;

void updateNodesX(
    std::map<std::string, XY> & positions,
    const ConnectionsMap &connections,
    const XY &lowerBound,
    const XY &upperBound
    )
{
    // convert to a bunch of lines
    std::vector<std::vector<std::string>> pyramid;
    for(float y=lowerBound.y; y<upperBound.y; y++){
        std::vector<std::string> layer;
        for(const auto & m : positions){
            if(m.second.y==y){
                layer.push_back(m.first);
            }
        }
        pyramid.push_back(layer);
    }

    // iterate over the lines, shuffling around X values for a bit
    bool done = false;
    int iter=0;
    while(!done && iter<1000){
        for(auto layer : pyramid){
            while(layer.size()){
                auto &p = layer.back(); layer.pop_back();
                for( auto &p_other : layer){
                    auto dx = positions[p].x - positions[p_other].x;
                    auto force = -powf(2.0f, -(50*dx)*(50*dx));
                    positions[p].x += force;
                    positions[p_other].x -= force;
                    
                }
            }
        }
        iter++;
    }

    // apply the re-arranged lines
}

void updateNodesY(
  const std::string & node, 
  std::map<std::string, XY> & positions,
  const ConnectionsMap & connections,
  int depth,
  const std::string & terminator
  )
{
    int currentDepth = positions[node].y;
    if(depth <= currentDepth) return;
    positions[node].y = depth;
    if(node == terminator) return;
    for(const auto &c : connections.at(node)){
        updateNodesY(c, positions, connections, depth-1, terminator);
    }
}

void GraphView::mouseDown(const MouseEvent & event) {
    for (const auto & mp : modulePosition) {
        float x1 = mp.second.x*gridSize;
        float x2 = mp.second.x*gridSize + nodeSize;
        float y1 = mp.second.y*gridSize;
        float y2 = mp.second.y*gridSize + nodeSize;
        if ( (event.x >= x1 ) 
          && (event.x <= x2 )
          && (event.y >= y1 )
          && (event.y <= y2 )
            ) 
        {
            clickedComponent = &(mp.first);
            beginDragAutoRepeat(100);
            break;
        }
    }
}

void GraphView::mouseDrag(const MouseEvent & event) {
    if (clickedComponent) {
        modulePosition[*clickedComponent].x = (event.x - 0.5*nodeSize) / gridSize;
        modulePosition[*clickedComponent].y = (event.y - 0.5*nodeSize) / gridSize;
        repaint();
    }
}

void GraphView::mouseUp(const MouseEvent & event) {
    clickedComponent = nullptr;
    recalculateBounds();
}

void GraphView::paint (Graphics& g){

    for (const auto &mpc : graphDescriptor.connections) {
        auto from = mpc.source.module;
        auto to = mpc.target.module;
        float x0 = modulePosition[from].x*gridSize + 0.5*nodeSize;
        float x1 = modulePosition[to].x*gridSize + 0.5*nodeSize;
        float y0 = modulePosition[from].y*gridSize + nodeSize;
        float y1 = modulePosition[to].y*gridSize;

        Path path;

        path.startNewSubPath(x0, y0);
        if (y1 <= y0) {
            float dy = 0.25*nodeSize;
            float dx = 0.25*nodeSize * (x1 >= x0 ? 1 : -1); 
            float s = x1 != x0 ? 1 : -1;
            float x[7]{
                x0+dx, 
                x0+2*dx,

                x0+3*dx, 
                x1-3*dx*s, 
                
                x1-2*dx*s,
                x1-dx*s, 
                x1
            };
            float y[7]{
                y0+2*dy, 
                y0+dy,

                y0, 
                y1, 

                y1-dy, 
                y1-2*dy, 
                y1
            };
            path.quadraticTo(x[0], y[0], x[1], y[1]);
            path.cubicTo(x[2], y[2], x[3], y[3], x[4], y[4]);
            path.quadraticTo(x[5], y[5], x[6], y[6]);
        }
        else {
            float d = 0.5*nodeSize;
            path.cubicTo( x0, y0+d, x1, y1-d, x1, y1 );
        }
        ColourGradient grad(Colours::yellow, x0, y0, Colours::red, x1, y1, false);
        g.setColour(Colours::green);
        PathStrokeType strokeType(1);
        strokeType.createStrokeWithArrowheads(path, path, 0, 0, 5, 5); // maybe no need
        g.setGradientFill(grad);
        g.fillPath(path);

    }

    int numInPorts = 3;
    int numOutPorts = 2;
    for(const auto &mp : modulePosition){
        float x = mp.second.x*gridSize;
        float y = mp.second.y*gridSize;
        float w = nodeSize;
        float h = nodeSize;
        g.setColour(Colour((uint8_t)0, (uint8_t)0, (uint8_t)0, (float)0.5f));
        g.fillRoundedRectangle(x, y, w, h, 5.f);

        g.setColour(Colours::white); // restore trans

        Path path;
        path.addRoundedRectangle(x, y, w, h, 5.f, 2.f);
        PathStrokeType strokeType(1);
        strokeType.createStrokedPath(path, path);
        ColourGradient grad(Colours::white, x, y, Colours::grey, x, y+h, false);
        g.setGradientFill(grad);
        g.fillPath(path);

        g.setColour(Colours::white);
        g.drawFittedText(mp.first, x, y, w, h, Justification::centred, 1);

        g.setGradientFill(grad);
        float r = 7.0f;
        for(float i=0; i<numInPorts; ++i){
            float x_ = x + ((i+.5f)/numInPorts) * nodeSize - r*.5f;
            g.fillEllipse(x_, y-r*0.5, r, r);
        }
        for(float i=0; i<numOutPorts; ++i){
            float x_ = x + ((i+0.5f)/numOutPorts) * nodeSize - r*.5f;;
            g.fillEllipse(x_, y+nodeSize-r*0.5, r, r);
        }
    }

}

void GraphView::resized() {
  repaint();
};

void GraphView::setGraph(const PhasePhckr::ConnectionGraphDescriptor& graph){
  graphDescriptor = graph;
  recalculate();
}

void GraphView::recalculate(){
  modulePosition.clear();

  // initial positions
  for(const auto &mv : graphDescriptor.modules){
    modulePosition[mv.name] = XY(0, INT_MIN);
  }

  // store the connections between nodes
  ConnectionsMap connections;
  for(const auto &mpc : graphDescriptor.connections){
    connections[mpc.source.module].insert(mpc.target.module);
    connections[mpc.target.module].insert(mpc.source.module);
  }

  const std::string start = "inBus";
  const std::string stop = "outBus";
  modulePosition[start] = XY(0, INT_MIN);
  modulePosition[stop] = XY(0, INT_MIN);

  // find all paths starting from inBus
  int depth = 0;
  updateNodesY(
      stop, modulePosition, connections, depth, start
  );
  
  // special case to make inBus always at the top
  for (auto const &p: modulePosition){
      if(p.first != start && p.second.y <= modulePosition[start].y){
          modulePosition[start].y = p.second.y - 1;
      }
  }

  // and shift everything to positive
  int y_bias = modulePosition[start].y;
  for (auto &p: modulePosition){
      p.second.y -= y_bias;
  }

  recalculateBounds();

  updateNodesX(modulePosition, connections, lowerBound, upperBound);

  int x_bias = INT_MAX;
  for (auto &p: modulePosition){
      if(p.second.x < x_bias){
          x_bias = p.second.x;
      }
  }

  for (auto &p: modulePosition){
     p.second.x -= x_bias;
  }

  recalculateBounds();

}

void GraphView::recalculateBounds() {
    bool boundChanged = false;
    for (const auto &mp : modulePosition) {
        const auto & p = mp.second;
        if (mp.second.x < lowerBound.x) {
            lowerBound.x = p.x;
            boundChanged = true;
        }
        if (mp.second.x > upperBound.x) {
            upperBound.x = p.x;
            boundChanged = true;
        }
        if (mp.second.y < lowerBound.y) {
            lowerBound.y = p.y;
            boundChanged = true;
        }
        if (mp.second.y > upperBound.y) {
            upperBound.y = p.y;
            boundChanged = true;
        }
    }

    if (boundChanged) {
        setBounds(
            0,
            0,
            (upperBound.x - lowerBound.x + 1)*gridSize,
            (upperBound.y - lowerBound.y + 1)*gridSize
        );
    }

}
