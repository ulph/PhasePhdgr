#include "GraphView.h"
#include <algorithm>
#include <utility>

typedef std::map<std::string, std::set<std::string>> ConnectionsMap;

using namespace PhasePhckr;


void GraphView::mouseDown(const MouseEvent & event) {
    parent.setScrollOnDragEnabled(true);
    for (const auto m : gfxGraph.modules) {
        if (m.within(XY(event.x, event.y))) {
            // do stuff
            return;
        }

        std::string name;
        bool inputPort;
        XY position;
        if (m.withinPort(XY(event.x, event.y), position, name, inputPort)) {
            // do stuff
            return;
        }
    }
}


void GraphView::mouseWheelMove(const MouseEvent & e, const MouseWheelDetails & d){
    //
}


void GraphView::mouseDrag(const MouseEvent & event) {
    lastMouse.x = event.x;
    lastMouse.y = event.y;
}


void GraphView::mouseUp(const MouseEvent & event) {
    //
}


void GraphView::mouseMove(const MouseEvent & event) {
    lastMouse.x = event.x;
    lastMouse.y = event.y;
}


void GraphView::updateBounds() {
    // lock before calling

    XY min(0, 0);
    XY max(0, 0);
    for (auto &mb : gfxGraph.modules) {
        if (mb.position.x+mb.size.x > max.x) {
            max.x = mb.position.x+c_GridSize;
        }
        if (mb.position.y+mb.size.y > max.y) {
            max.y = mb.position.y+c_GridSize;
        }
        if (mb.position.x-c_NodeSize < min.x) {
            min.x = mb.position.x-c_NodeSize;
        }
        if (mb.position.y-c_NodeSize < min.y) {
            min.y = mb.position.y-c_NodeSize;
        }
    }

    if (min.y < 0) {
        for (auto &mb : gfxGraph.modules) {
            mb.position += XY(0, -min.y);
            mb.resizeAndRepositionPorts();
        }
        for (auto &w : gfxGraph.wires) {
            w.position += XY(0, -min.y);
        }
        max.y -= min.y;
    }

    if (min.x < 0) {
        for (auto &mb : gfxGraph.modules) {
            mb.position += XY(-min.x, 0);
            mb.resizeAndRepositionPorts();
        }
        for (auto &w : gfxGraph.wires) {
            w.position += XY(-min.x, 0);
        }
        max.x -= min.x;
    }

    auto bounds = getBounds();
    if (bounds.getWidth() < max.x) {
        bounds.setWidth(max.x);
    }
    if (bounds.getHeight() < max.y) {
        bounds.setHeight(max.y);
    }
    setBounds(bounds);

}


void GraphView::paint (Graphics& g){
    while (renderLock.test_and_set(std::memory_order_acquire));
    updateBounds();
    for(auto &w : gfxGraph.wires){
        w.draw(g);
    }
    for (auto &mb : gfxGraph.modules) {
        mb.draw(g);
    }
    if (looseWire) {
        looseWire->draw(g);
    }
    renderLock.clear(std::memory_order_release);
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
    if (depth <= currentDepth) return;
    positions[node].y = depth;
    if (node == terminator) return;
    for (const auto &c : connections.at(node)) {
        updateNodesY(c, positions, connections, depth - 1, terminator);
    }
}


void GraphView::setGraph(const ConnectionGraphDescriptor& graph) {
    while (dataLock.test_and_set(std::memory_order_acquire));
    connectionGraphDescriptor = graph;
    dataLock.clear(std::memory_order_release);
    prepareRenderComponents();
}


void GraphView::prepareRenderComponents(){

  while (dataLock.test_and_set(std::memory_order_acquire));

  std::map<std::string, XY> modulePositions;

  // initial positions
  for(const auto &mv : connectionGraphDescriptor.modules){
    modulePositions[mv.name] = XY(0, INT_MIN);
  }

  // store the connections between nodes
  ConnectionsMap connections;
  for(const auto &mpc : connectionGraphDescriptor.connections){
    connections[mpc.source.module].insert(mpc.target.module);
    connections[mpc.target.module].insert(mpc.source.module);
  }

  const std::string start = inBus.name;
  const std::string stop = outBus.name;
  modulePositions[start] = XY(0, INT_MIN);
  modulePositions[stop] = XY(0, INT_MIN);

  // find all paths starting from inBus
  int depth = 0;
  updateNodesY(
      stop, modulePositions, connections, depth, start
  );
  
  // special case to make inBus always at the top
  for (auto const &p: modulePositions){
      if(p.first != start && p.second.y <= modulePositions[start].y){
          modulePositions[start].y = p.second.y - 1;
      }
  }

  // and shift everything to positive
  int y_bias = modulePositions[start].y;
  for (auto &p: modulePositions){
      p.second.y -= y_bias;
  }

  // TODO -- traverse X as well

  auto cgd = connectionGraphDescriptor;

  dataLock.clear(std::memory_order_release);

  createRenderComponents(modulePositions);

}

void GraphView::createRenderComponents(const std::map<std::string, XY> & modulePositions) {
    // create the renderable structures
    while (dataLock.test_and_set(std::memory_order_acquire));
    while (renderLock.test_and_set(std::memory_order_acquire));

    gfxGraph = GfxGraph();

    std::vector<ModuleVariable> modules = connectionGraphDescriptor.modules; 
    modules.push_back(inBus);
    modules.push_back(outBus);
    for (const auto & m : modules) {
        gfxGraph.modules.emplace_back(
            GfxModule(
                m, 
                modulePositions.at(m.name).x, 
                modulePositions.at(m.name).y, 
                1.0, 
                1.0, 
                doc, 
                connectionGraphDescriptor.values
            )
        );
    }

    for (const auto & m : connectionGraphDescriptor.connections) {
        /*
        gfxGraph.wires.emplace_back(
            GfxWire()
        );
        */
    }

    renderLock.clear(std::memory_order_release);
    dataLock.clear(std::memory_order_release);
}
