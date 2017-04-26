#include "GraphView.h"
#include <algorithm>
#include <utility>

typedef std::map<std::string, std::set<std::string>> ConnectionsMap;

using namespace PhasePhckr;

void GraphView::mouseDown(const MouseEvent & event) {
    parent.setScrollOnDragEnabled(true);
    while (userActionLock.test_and_set(std::memory_order_acquire));
    for (const auto m : gfxGraph_userActionCopy.modules) {
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
    userActionLock.clear(std::memory_order_release);
}


void GraphView::mouseWheelMove(const MouseEvent & e, const MouseWheelDetails & d){
    while (userActionLock.test_and_set(std::memory_order_acquire));
    userActionLock.clear(std::memory_order_release);
}


void GraphView::mouseDrag(const MouseEvent & event) {
    while (userActionLock.test_and_set(std::memory_order_acquire));
    lastMouse.x = event.x;
    lastMouse.y = event.y;
    userActionLock.clear(std::memory_order_release);
}


void GraphView::mouseUp(const MouseEvent & event) {
    while (userActionLock.test_and_set(std::memory_order_acquire));
    userActionLock.clear(std::memory_order_release);
}


void GraphView::mouseMove(const MouseEvent & event) {
    while (userActionLock.test_and_set(std::memory_order_acquire));
    lastMouse.x = event.x;
    lastMouse.y = event.y;
    userActionLock.clear(std::memory_order_release);
}


void GraphView::updateBounds() {
    XY min(0, 0);
    XY max(0, 0);
    for (auto &mb : gfxGraph_renderCopy.modules) {
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
        for (auto &mb : gfxGraph_renderCopy.modules) {
            mb.position += XY(0, -min.y);
            mb.resizeAndRepositionPorts();
        }
        for (auto &w : gfxGraph_renderCopy.wires) {
            w.position += XY(0, -min.y);
            w.destination += XY(0, -min.y);
        }
        max.y -= min.y;
    }

    if (min.x < 0) {
        for (auto &mb : gfxGraph_renderCopy.modules) {
            mb.position += XY(-min.x, 0);
            mb.resizeAndRepositionPorts();
        }
        for (auto &w : gfxGraph_renderCopy.wires) {
            w.position += XY(-min.x, 0);
            w.destination += XY(-min.x, 0);
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
    for(auto &w : gfxGraph_renderCopy.wires){
        w.draw(g);
    }
    for (auto &mb : gfxGraph_renderCopy.modules) {
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
    while (connectionGraphDescriptorLock.test_and_set(std::memory_order_acquire));
    connectionGraphDescriptor = graph;
    connectionGraphDescriptorLock.clear(std::memory_order_release);
    prepareRenderComponents();
}


void GraphView::prepareRenderComponents(){

  while (connectionGraphDescriptorLock.test_and_set(std::memory_order_acquire));
  connectionGraphDescriptor.modules.push_back(inBus);
  connectionGraphDescriptor.modules.push_back(outBus);
  auto cgd = connectionGraphDescriptor;
  connectionGraphDescriptorLock.clear(std::memory_order_release);

  // (convinience) store the connections between nodes
  ConnectionsMap connections;
  for (const auto &mpc : cgd.connections) {
      connections[mpc.source.module].insert(mpc.target.module);
      connections[mpc.target.module].insert(mpc.source.module);
  }

  while (modulePositionsLock.test_and_set(std::memory_order_acquire));
  // initial positions
  for(const auto &mv : cgd.modules){
    modulePositions[mv.name] = XY(0, INT_MIN);
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

  auto mp = modulePositions;
  modulePositionsLock.clear(std::memory_order_release);

  updateRenderComponents(cgd, mp);

}


void GraphView::updateRenderComponents(const ConnectionGraphDescriptor &cgd_copy, const std::map<std::string, XY> & mp_copy) {
    // create the renderable structures

    GfxGraph gfxGraph = GfxGraph();

    for (const auto & m : cgd_copy.modules) {
        gfxGraph.modules.emplace_back(
            GfxModule(
                m, 
                mp_copy.at(m.name).x,
                mp_copy.at(m.name).y,
                1.0, 
                1.0, 
                doc, 
                cgd_copy.values
            )
        );
    }

    for (const auto & c : cgd_copy.connections) {
        gfxGraph.wires.emplace_back(
            GfxWire(c, gfxGraph.modules)
        );
    }

    while (renderLock.test_and_set(std::memory_order_acquire));
    gfxGraph_renderCopy = gfxGraph;
    renderLock.clear(std::memory_order_release);

    while (userActionLock.test_and_set(std::memory_order_acquire));
    gfxGraph_userActionCopy = gfxGraph;
    userActionLock.clear(std::memory_order_release);

}
