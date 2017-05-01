#pragma once

#include <string>
#include "GraphViewGfx.hpp"
#include "design.hpp"
#include "GraphViewNodeStuff.hpp"
#include <queue>
#include <set>
#include <assert.h>
#include <cfloat>

using namespace std;

void DFSfindDepths(
    string node,
    ModulePositionMap & positions,
    const ConnectionsMap & connections,
    int depth,
    const std::string & terminator
)
{
    int currentDepth = (int)positions[node].y;
    if (depth <= currentDepth) return;
    positions[node].y = (float)depth;
    if (node == terminator) return;
    auto it = connections.find(node);
    if (it == connections.end()) return;
    for (const auto &c : it->second) {
        DFSfindDepths(c, positions, connections, depth - 1, terminator);
    }
}


void BFSfindDepths(
    const string & n_init,
    ModulePositionMap & positions,
    const ConnectionsMap & connections,
    const int d_init,
    const string & terminator
) {
    queue<pair<string,int>> nextNodes;
    nextNodes.push(make_pair(n_init, d_init));
    while (nextNodes.size()) {
        auto p = nextNodes.front(); nextNodes.pop();
        auto n = p.first;
        auto d = p.second;
        if (d <= positions[n].y) continue;
        positions[n].y = (float)d;
        if (n == terminator) continue;
        auto it = connections.find(n);
        if (it == connections.end()) continue;
        for (const auto &c : it->second) {
            nextNodes.push(make_pair(c, d - 1));
        }
    }
}


vector<string> findLongestPathY(
    ModulePositionMap & modulePositions,
    const ConnectionsMap & undirectedConnections,
    const ConnectionsMap & backwardsConnections,
    const ConnectionsMap & forwardsConnections,
    const string & start,
    const string & stop
) {
    /*
    y coordinate for a node is defined as it's distance to 'stop'
    */

    /*
    DFSfindPathsY(
        stop, 
        modulePositions, 
        undirectedConnections, 
        0, 
        start
    );
    */

    BFSfindDepths(
        stop,
        modulePositions, 
        undirectedConnections,
        0, 
        start
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

    // now, we can iterate from the top
    vector<string> path;
    string n = start;
    while (true) {
        path.push_back(n);
        float min_y = FLT_MAX;
        auto itn = forwardsConnections.find(n);
        // check if we can continue (graph may be broken)
        if (itn == forwardsConnections.end()) break;
        // find the next node which as the lowest y (utilizing the topology)
        for (const auto & c : itn->second) {
            if (min_y > modulePositions[c].y) {
                min_y = modulePositions[c].y;
                n = c;
            }
        }
    }
    return path;
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
    ConnectionsMap forwardsConnections;
    for (const auto &mpc : connectionGraphDescriptor.connections) {
        undirectedConnections[mpc.source.module].insert(mpc.target.module);
        undirectedConnections[mpc.target.module].insert(mpc.source.module);
        backwardsConnections[mpc.target.module].insert(mpc.source.module);
        forwardsConnections[mpc.source.module].insert(mpc.target.module);
    }

    // initial positions
    for (const auto &mv : connectionGraphDescriptor.modules) {
        modulePositions[mv.name] = XY(0, INT_MIN);
    }
    modulePositions[start] = XY(0, INT_MIN);
    modulePositions[stop] = XY(0, INT_MIN);

    // find all y positions and return the longest path
    auto path = findLongestPathY(modulePositions, undirectedConnections, backwardsConnections, forwardsConnections, start, stop);

    // set all x along the longest path to 0

    // find all other x positions
    setNodeX(modulePositions, backwardsConnections, start, stop);
}
