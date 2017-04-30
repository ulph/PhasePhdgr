#pragma once

#include <string>
#include <unordered_map>
#include "GraphViewGfx.hpp"
#include "design.hpp"
#include <unordered_set>
#include "GraphViewNodeStuff.hpp"

using namespace std;

void updateNodesY(
    const string & node,
    ModulePositionMap & positions,
    const ConnectionsMap & undirectedConnections,
    int depth,
    const std::string & terminator
)
{
    int currentDepth = (int)positions[node].y;
    if (depth <= currentDepth) return;
    positions[node].y = (float)depth;
    if (node == terminator) return;
    for (const auto &c : undirectedConnections.at(node)) {
        updateNodesY(c, positions, undirectedConnections, depth - 1, terminator);
    }
}


void setNodeY(
    ModulePositionMap & modulePositions,
    const ConnectionsMap & undirectedConnections,
    const string & start,
    const string & stop
) {
    /*
    y coordinate for a node is defined as it's distance to 'stop'
    */

    int depth = 0;
    updateNodesY(
        stop, modulePositions, undirectedConnections, depth, start
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
    ModulePositionMap & modulePositions,
    const ConnectionsMap & backwardsConnections,
    const string & start,
    const string & stop
) {
    /*
    x coordinate for a node is defined as number of branches from the centre path
    */
}


void setNodePositions(
    const ConnectionGraphDescriptor& connectionGraphDescriptor,
    ModulePositionMap & modulePositions,
    const string &start,
    const string &stop
) {
    // (convinience) store the connections between nodes
    ConnectionsMap undirectedConnections;
    ConnectionsMap backwardsConnections;
    for (const auto &mpc : connectionGraphDescriptor.connections) {
        undirectedConnections[mpc.source.module].insert(mpc.target.module);
        undirectedConnections[mpc.target.module].insert(mpc.source.module);
        backwardsConnections[mpc.source.module].insert(mpc.target.module);
    }

    // initial positions
    for (const auto &mv : connectionGraphDescriptor.modules) {
        modulePositions[mv.name] = XY(0, INT_MIN);
    }
    modulePositions[start] = XY(0, INT_MIN);
    modulePositions[stop] = XY(0, INT_MIN);

    // find all y positions
    setNodeY(modulePositions, undirectedConnections, start, stop);

    // find all x positions
    setNodeX(modulePositions, backwardsConnections, start, stop);
}
