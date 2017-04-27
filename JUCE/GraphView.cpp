#include "GraphView.h"
#include <algorithm>
#include <utility>

typedef std::map<std::string, std::set<std::string>> ConnectionsMap;

using namespace PhasePhckr;


void GraphView::propagateUserModelChange() {

    repaint();
}


void GraphView::mouseDown(const MouseEvent & event) {
    parent.setScrollOnDragEnabled(true);
    while (gfxGraphLock.test_and_set(std::memory_order_acquire));
    for (auto & m : gfxGraph.modules) {
        std::string port;
        bool inputPort;
        XY position;
        if (m.withinPort(XY(event.x, event.y), position, port, inputPort)) {
            delete looseWire;
            looseWire = new GfxLooseWire();
            looseWire->position = position;
            looseWire->destination.x = event.x;
            looseWire->destination.y = event.y;
            looseWire->attachedAtSource = !inputPort;
            looseWire->attachedPort = { m.module.name, port };
            parent.setScrollOnDragEnabled(false);
            break;
        }
        if (m.within(XY(event.x, event.y))) {
            draggedModule = &m;
            parent.setScrollOnDragEnabled(false);
            break;
        }
    }
    for (auto w : gfxGraph.wires) {
        // TODO - grap a wire
    }
    gfxGraphLock.clear(std::memory_order_release);
}


void GraphView::mouseWheelMove(const MouseEvent & e, const MouseWheelDetails & d){

}


void GraphView::mouseDrag(const MouseEvent & event) {
    bool modelChanged = false;
    while (gfxGraphLock.test_and_set(std::memory_order_acquire));
    if (draggedModule) {
        modelChanged = true;
        draggedModule->position.x = event.x - draggedModule->size.x * 0.5;
        draggedModule->position.y = event.y - draggedModule->size.y * 0.5;
        draggedModule->repositionPorts();
        auto mv = std::vector<GfxModule>{ *draggedModule };
        for (auto w : gfxGraph.wires) {
            w.calculatePath(mv);
        }
    }
    if (looseWire) {
        looseWire->destination.x = event.x;
        looseWire->destination.y = event.y;
    }
    gfxGraphLock.clear(std::memory_order_release);
    if (modelChanged) propagateUserModelChange();
}


void GraphView::mouseUp(const MouseEvent & event) {
    while (gfxGraphLock.test_and_set(std::memory_order_acquire));
    bool modelChanged = false;
    draggedModule = nullptr;
    if (looseWire) {
        for (auto m : gfxGraph.modules) {
            // do stuff
        }
    }
    delete looseWire;
    gfxGraphLock.clear(std::memory_order_release);
    if (modelChanged) propagateUserModelChange();
}


void GraphView::mouseMove(const MouseEvent & event) {

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
    while (gfxGraphLock.test_and_set(std::memory_order_acquire));

    g.fillAll(Colours::black);

    for(auto &w : gfxGraph.wires){
        w.draw(g);
    }
    for (auto &mb : gfxGraph.modules) {
        mb.draw(g);
    }
    if (looseWire) {
        looseWire->draw(g);
    }

    gfxGraphLock.clear(std::memory_order_release);
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
  std::map<std::string, XY> modulePositions;

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

  updateRenderComponents(cgd, modulePositions);

}


void GraphView::updateRenderComponents(const ConnectionGraphDescriptor &cgd_copy, const std::map<std::string, XY> & mp) {
    // create the renderable structures

    while (gfxGraphLock.test_and_set(std::memory_order_acquire));

    gfxGraph = GfxGraph();

    for (const auto & m : cgd_copy.modules) {
        gfxGraph.modules.emplace_back(
            GfxModule(
                m, 
                mp.at(m.name).x,
                mp.at(m.name).y,
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

    gfxGraphLock.clear(std::memory_order_release);

    updateBounds(bounds);

}
