/*
  ==============================================================================

    GraphView.cpp
    Created: 20 Apr 2017 4:37:23pm
    Author:  ulph

  ==============================================================================
*/

#include "GraphView.h"
#include <algorithm>

typedef std::map<std::string, std::set<std::string>> ConnectionsMap;

struct PathAndCost{
    std::vector<std::string> path;
    int cost;
};

std::vector<PathAndCost> updateNodesX(
    const PathAndCost &path, 
    std::map<std::string, XY> & positions,
    const ConnectionsMap &connections
    )
{
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
        auto from = mpc.from.module;
        auto to = mpc.to.module;
        float x0 = modulePosition[from].x*gridSize + 0.5*nodeSize;
        float x1 = modulePosition[to].x*gridSize + 0.5*nodeSize;
        float y0 = modulePosition[from].y*gridSize + nodeSize;
        float y1 = modulePosition[to].y*gridSize;

        Path path;
        PathStrokeType strokeType(1);

        path.startNewSubPath(x0, y0);
        if (y1 <= y0) {
            g.setColour(Colours::darkgrey);
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
            g.setColour(Colours::grey);
            path.cubicTo( x0, y0+d, x1, y1-d, x1, y1 );
        }

        g.strokePath(path, strokeType);

    }

    g.setColour(Colours::white);
    for(const auto &mp : modulePosition){
        float x = mp.second.x*gridSize;
        float y = mp.second.y*gridSize;
        float w = nodeSize;
        float h = nodeSize;
        g.drawRoundedRectangle(x, y, w, h, 5.f, 2.f);
        g.drawFittedText(mp.first, x, y, w, h, Justification::centred, 1);
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
    connections[mpc.from.module].insert(mpc.to.module);
    connections[mpc.to.module].insert(mpc.from.module);
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
  int bias = modulePosition[start].y;
  for (auto &p: modulePosition){
      p.second.y -= bias;
  } 

  recalculateBounds();

}

void GraphView::recalculateBounds() {
    bool boundChanged = false;
    for (const auto &mp : modulePosition) {
        if (mp.second.x < x_bounds.first) {
            x_bounds.first = mp.second.x;
            boundChanged = true;
        }
        if (mp.second.x > x_bounds.second) {
            x_bounds.second = mp.second.x;
            boundChanged = true;
        }
        if (mp.second.y < y_bounds.first) {
            y_bounds.first = mp.second.y;
            boundChanged = true;
        }
        if (mp.second.y > y_bounds.second) {
            y_bounds.second = mp.second.y;
            boundChanged = true;
        }
    }

    if (boundChanged) {
        setBounds(
            0,
            0,
            (x_bounds.second - x_bounds.first + 1)*gridSize,
            (y_bounds.second - y_bounds.first + 1)*gridSize
        );
    }

}