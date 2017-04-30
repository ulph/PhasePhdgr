#include <string>
#include <map>
#include "GraphViewGfx.hpp"
#include "design.hpp"
#include <set>

using namespace std;

typedef map<string, set<string>> ConnectionsMap;

void updateNodesY(
    const string & node,
    map<string, XY> & positions,
    const ConnectionsMap & connections,
    int depth,
    const std::string & terminator
)
{
    int currentDepth = (int)positions[node].y;
    if (depth <= currentDepth) return;
    positions[node].y = (float)depth;
    if (node == terminator) return;
    for (const auto &c : connections.at(node)) {
        updateNodesY(c, positions, connections, depth - 1, terminator);
    }
}

void setNodeY(
    map<string, XY> & modulePositions,
    const ConnectionsMap & connections,
    const string & start,
    const string & stop
) {

    // recursively find the y coordinates for each node - as defined as distance from outBus
    int depth = 0;
    updateNodesY(
        stop, modulePositions, connections, depth, start
    );

    // special case to make inBus always at the top
    for (auto const &p : modulePositions) {
        if (p.first != start && p.second.y <= modulePositions[start].y) {
            modulePositions[start].y = p.second.y - 1;
        }
    }

    // and shift everything to positive
    float y_bias = modulePositions[start].y;
    for (auto &p : modulePositions) {
        p.second.y -= y_bias;
    }
}


void setNodeX(
    map<string, XY> & modulePositions,
    const ConnectionsMap & connections,
    const string & start,
    const string & stop
) {

}


void setNodePositions(
    const ConnectionGraphDescriptor& connectionGraphDescriptor,
    map<string, XY> & modulePositions,
    const string &start,
    const string &stop
) {
    // (convinience) store the connections between nodes
    ConnectionsMap connections;
    for (const auto &mpc : connectionGraphDescriptor.connections) {
        connections[mpc.source.module].insert(mpc.target.module);
        connections[mpc.target.module].insert(mpc.source.module);
    }

    // initial positions
    for (const auto &mv : connectionGraphDescriptor.modules) {
        modulePositions[mv.name] = XY(0, INT_MIN);
    }

    modulePositions[start] = XY(0, INT_MIN);
    modulePositions[stop] = XY(0, INT_MIN);

    // find all y positions
    setNodeY(modulePositions, connections, start, stop);

    // find all x positions
    setNodeX(modulePositions, connections, start, stop);
}
