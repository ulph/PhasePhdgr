#include "GraphView.h"
#include <algorithm>
#include <utility>

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

static bool withinPort(float mx, float my, const std::map<std::string, XY>& portPositions, std::string & portName, float x1, float y1, float r, float scale) {
    for (const auto & pp : portPositions) {
        float xp1 = x1 + pp.second.x*scale;
        float xp2 = xp1 + r*scale;
        float yp1 = y1 + pp.second.y*scale;
        float yp2 = yp1 + r*scale;
        if ((mx >= xp1)
            && (mx <= xp2)
            && (my >= yp1)
            && (my <= yp2)
        ) {
            portName = pp.first;
            return true;
        }
    }
    return false;
}

void GraphView::mouseDown(const MouseEvent & event) {
    parent.setScrollOnDragEnabled(true);
    for (const auto & mp : modulePosition) {

        float ws = 1.0f;
        if (moduleWidthScale.count(mp.first)) ws = moduleWidthScale.at(mp.first);

        float x1 = scale*(mp.second.x*gridSize - r);
        float x2 = scale*(mp.second.x*gridSize + ws*nodeSize + r);
        float y1 = scale*(mp.second.y*gridSize - r);
        float y2 = scale*(mp.second.y*gridSize + nodeSize + r);
        if ( (event.x >= x1 ) 
          && (event.x <= x2 )
          && (event.y >= y1 )
          && (event.y <= y2 )
            ) 
        {

            if (inputPortPositions.count(mp.first)){
                const auto & pp = inputPortPositions.at(mp.first);
                std::string portName;
                if (withinPort(event.x, event.y, pp, portName, x1+scale*r, y1+scale*r, r, scale)) {
                    delete connectionInProgress;
                    connectionInProgress = new Wire();
                    connectionInProgress->port = make_pair(mp.first, portName);
                    connectionInProgress->state = Wire::attachedToInput;
                    parent.setScrollOnDragEnabled(false);
                    return;
                }
            }

            if (outputPortPositions.count(mp.first)) {
                const auto & pp = outputPortPositions.at(mp.first);
                std::string portName;
                if (withinPort(event.x, event.y, pp, portName, x1+scale*r, y1+scale*r, r, scale)) {
                    delete connectionInProgress;
                    connectionInProgress = new Wire();
                    connectionInProgress->port = make_pair(mp.first, portName);
                    connectionInProgress->state = Wire::attachedToOutput;
                    parent.setScrollOnDragEnabled(false);
                    return;
                }
            }

            parent.setScrollOnDragEnabled(false);
            clickedComponent = &(mp.first);
            return;

        }
    }
}

void GraphView::mouseWheelMove(const MouseEvent & e, const MouseWheelDetails & d){
    if(d.deltaY){
        scale += d.deltaY>0?0.05:-0.05;
        recalculateBounds(true);
        repaint();
    }
}

void GraphView::mouseDrag(const MouseEvent & event) {
    lastMouse.x = event.x;
    lastMouse.y = event.y;

    if (connectionInProgress) {
        repaint();
    }
    else if (clickedComponent) {
        float ws = 1.0f;
        if (moduleWidthScale.count(*clickedComponent)) ws = moduleWidthScale.at(*clickedComponent);
        modulePosition[*clickedComponent].x = (event.x/scale - 0.5*nodeSize*ws) / gridSize;
        modulePosition[*clickedComponent].y = (event.y/scale - 0.5*nodeSize) / gridSize;
        recalculateBounds();
        repaint();
    }
}

void GraphView::mouseUp(const MouseEvent & event) {
    clickedComponent = nullptr;
    delete connectionInProgress;
    connectionInProgress = nullptr;
    repaint();
}

void GraphView::mouseMove(const MouseEvent & event) {
    lastMouse.x = event.x;
    lastMouse.y = event.y;
}

static void drawCable(Graphics& g, float x0, float y0, float x1, float y1, float r, float nodeSize, float s, bool wire=false){
    Path path;
    
    x0 += 0.5f*r;
    y0 += 0.5f*r;
    x1 += 0.5f*r;
    y1 += 0.5f*r;

    x0 *= s;
    y0 *= s;
    x1 *= s;
    y1 *= s;

    nodeSize *= s;

    g.setColour(Colours::green);
    PathStrokeType strokeType(1);

    path.startNewSubPath(x0, y0);
    if (wire) {
        path.lineTo(x1, y1);
    }
    else if (y1 <= y0) {
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
    strokeType.createStrokedPath(path, path);
    if (!wire) {
        ColourGradient grad(Colours::yellow, x0, y0, Colours::red, x1, y1, false);
        g.setGradientFill(grad);
    }
    g.fillPath(path);
}


static void drawModule(Graphics& g, float x, float y, float w, float h, std::string label, float nodeSize, float s){
    x *= s;
    y *= s;
    w *= s;
    h *= s;
    nodeSize *= s;

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
    g.drawFittedText(label, x, y, w, h, Justification::centred, 1);
}

static void drawModuleBundle(
    Graphics& g, 
    float x, 
    float y, 
    float r, 
    float w,
    float h,
    float nodeSize,
    const std::string & name,
    const std::string & type,
    const std::map< std::string, std::map<std::string, XY>> & inputPortPositions,
    const std::map< std::string, std::map<std::string, XY>> & outputPortPositions,
    float s
) {

    drawModule(g, x, y, w, h, name + "\n\n" + type, nodeSize, s);

    x *= s;
    y *= s;
    r *= s;

    if (inputPortPositions.count(name)) {
        for (const auto pp : inputPortPositions.at(name)) {
            g.setColour(Colours::black);
            g.fillEllipse(x + s*pp.second.x, y + s*pp.second.y, r, r);
            g.setColour(Colours::grey);
            g.drawEllipse(x + s*pp.second.x, y + s*pp.second.y, r, r, 1.0f);
            // draw label
        }
    }
    if (outputPortPositions.count(name)) {
        for (const auto pp : outputPortPositions.at(name)) {
            g.setColour(Colours::black);
            g.fillEllipse(x + s*pp.second.x, y + s*pp.second.y, r, r);
            g.setColour(Colours::grey);
            g.drawEllipse(x + s*pp.second.x, y + s*pp.second.y, r, r, 1.0f);
            // draw label
        }
    }
}

void GraphView::paint (Graphics& g){

    for (const auto& mpc : graphDescriptor.connections) {
        if(modulePosition.count(mpc.source.module) && modulePosition.count(mpc.target.module)){
            const auto& from = mpc.source.module;
            const auto& to = mpc.target.module;
            float x0 = modulePosition.at(from).x * gridSize;
            float y0 = modulePosition.at(from).y * gridSize;
            float x1 = modulePosition.at(to).x * gridSize;
            float y1 = modulePosition.at(to).y * gridSize;
            if(outputPortPositions.count(from) && inputPortPositions.count(to)){
                const auto& fromMap = outputPortPositions.at(from);
                const auto& toMap = inputPortPositions.at(to);
                if(fromMap.count(mpc.source.port) && toMap.count(mpc.target.port)){
                    const auto& fromPos = fromMap.at(mpc.source.port);
                    const auto& toPos = toMap.at(mpc.target.port);
                    drawCable(g, x0+fromPos.x, y0+fromPos.y, x1+toPos.x, y1+toPos.y, r, nodeSize, scale);
                }
            }
        }
    }

    for(const auto &m : graphDescriptor.modules){
        if(modulePosition.count(m.name)){
            const auto& pos = modulePosition.at(m.name);
            float x = pos.x*gridSize;
            float y = pos.y*gridSize;
            float ws = 1.0f;
            if (moduleWidthScale.count(m.name)) ws = moduleWidthScale.at(m.name);
            drawModuleBundle(g, x, y, r, ws*nodeSize, nodeSize, nodeSize, m.name, m.type, inputPortPositions, outputPortPositions, scale);
        }
    }

    std::vector< std::pair<std::string, std::string>> busModules;
    busModules.push_back(inBus);
    busModules.push_back(outBus);
    for (const auto &p : busModules) {
        if (modulePosition.count(p.first)) {
            const auto& pos = modulePosition.at(p.first);
            float x = pos.x*gridSize;
            float y = pos.y*gridSize;
            float ws = 1.0f;
            if (moduleWidthScale.count(p.first)) ws = moduleWidthScale.at(p.first);
            drawModuleBundle(g, x, y, r, ws*nodeSize, nodeSize, nodeSize, p.first, p.second, inputPortPositions, outputPortPositions, scale);
        }
    }

    if (connectionInProgress) {
        auto * pp = &outputPortPositions;
        if (connectionInProgress->state == Wire::attachedToInput) pp = &inputPortPositions;
        const auto &m = connectionInProgress->port.first;
        const auto &p = connectionInProgress->port.second;
        if (pp->count(m) && modulePosition.count(m)) {
            if (pp->at(m).count(p)) {
                float x0 = modulePosition.at(m).x * gridSize + pp->at(m).at(p).x;
                float y0 = modulePosition.at(m).y * gridSize + pp->at(m).at(p).y;
                float x1 = lastMouse.x;
                float y1 = lastMouse.y;
                drawCable(g, x0, y0, x1, y1, r, nodeSize, scale, true);
            }
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

static void buildPortPositions(
    std::map< std::string, std::map<std::string, XY>> & inputPortPositions,
    std::map< std::string, std::map<std::string, XY>> & outputPortPositions,
    const std::string & name,
    const ModuleDoc & doc,
    float w,
    float h,
    float r,
    bool busModule
    ) {
    float i = 0;
    std::map<std::string, XY> ipos;
    for (const auto& ip : doc.inputs) {
        ipos[ip.name] = XY((i + 0.5f) / (doc.inputs.size())*w * (busModule ? 4 : 1) - 0.5f*r, -0.5f*r);
        i++;
    }
    inputPortPositions[name] = ipos;
    i = 0;
    std::map<std::string, XY> opos;
    for (const auto& op : doc.outputs) {
        opos[op.name] = XY((i + 0.5f) / (doc.outputs.size())*w * (busModule ? 4 : 1) - 0.5f*r, h - 0.5f*r);
        i++;
    }
    outputPortPositions[name] = opos;
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

  const std::string start = inBus.first;
  const std::string stop = outBus.first;
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

  int x_min = INT_MAX;
  int x_max = INT_MIN;
  for (auto &p: modulePosition){
      if(p.second.x < x_min){
          x_min = p.second.x;
      }
  }

  x_max -= x_min;

  x_max = x_max > 5 ? x_max : 5;

  float x_move = -x_min + 0.5*x_max;
  for (auto &p: modulePosition){
     p.second.x += x_move;
  }

  // scale the horizontal scale if needed
  moduleWidthScale[inBus.first] = 4.0f;
  moduleWidthScale[outBus.first] = 4.0f;

  // calculate the port positions

  const auto& docs = doc.get();

  for(const auto &m : graphDescriptor.modules){
      if(docs.count(m.type)){
          const auto& doc = docs.at(m.type);
          buildPortPositions(inputPortPositions, outputPortPositions, m.name, doc, nodeSize, nodeSize, r, false);
      }
  }

  std::vector< std::pair<std::string, std::string>> busModules;
  busModules.push_back(inBus);
  busModules.push_back(outBus);
  for (const auto &p : busModules) {
      if (docs.count(p.second)) {
          const auto& doc = docs.at(p.second);
          buildPortPositions(inputPortPositions, outputPortPositions, p.first, doc, nodeSize, nodeSize, r, true);
      }
  }

  recalculateBounds(true);

}


void GraphView::recalculateBounds(bool force) {
    bool boundChanged = force;
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

    auto p = parent.getViewPosition();

    if (lowerBound.x < 0) {
        for (auto &mp : modulePosition) {
            mp.second.x -= lowerBound.x;
        }
        p.addXY(-gridSize*lowerBound.x*scale, 0);
        lowerBound.x = 0;
    }

    if (lowerBound.y < 0) {
        for (auto &mp : modulePosition) {
            mp.second.y -= lowerBound.y;
        }
        p.addXY(0, -gridSize*lowerBound.y*scale);
        lowerBound.y = 0;
    }

    if (boundChanged) {
        setBounds(
            lowerBound.x*gridSize*scale,
            lowerBound.y*gridSize*scale,
            (upperBound.x + 4)*gridSize*scale,
            (upperBound.y + 1)*gridSize*scale
        );
    }

    parent.setViewPosition(p);

}
