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
            break;
        }

        std::string name;
        bool inputPort;
        XY position;
        if (m.withinPort(XY(event.x, event.y), position, name, inputPort)) {
            // do stuff
            break;
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


void GraphView::updateBounds(const std::pair<XY, XY>& rectangle) {
    updateBounds(rectangle.first, rectangle.second);
}


void GraphView::updateBounds(const XY & position, const XY & size){
    auto bounds = getBounds();
    if (bounds.getWidth() < (position.x + size.x)) {
        bounds.setWidth(position.x + size.x);
    }
    if (bounds.getHeight() < (position.y + size.y)) {
        bounds.setHeight(position.y + size.y);
    }
    setBounds(bounds);
}


void GraphView::paint (Graphics& g){
    while (renderLock.test_and_set(std::memory_order_acquire));

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

  float halfSreen = getWidth() * 0.5;

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

    auto bounds = gfxGraph.getBounds();
    XY delta(0, 0);
    if (bounds.first.x < 0) delta.x = -bounds.first.x;
    if (bounds.first.y < 0) delta.y = -bounds.first.y;
    if (delta) {
        gfxGraph.moveDelta(delta);
        bounds = gfxGraph.getBounds();
    }
    updateBounds(bounds);

    while (renderLock.test_and_set(std::memory_order_acquire));
    gfxGraph_renderCopy = gfxGraph;
    renderLock.clear(std::memory_order_release);

    while (userActionLock.test_and_set(std::memory_order_acquire));
    gfxGraph_userActionCopy = gfxGraph;
    userActionLock.clear(std::memory_order_release);

}
