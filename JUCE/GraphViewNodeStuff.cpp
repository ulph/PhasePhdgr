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

const float initial_x = 20; // cannot be infinite or JUCE gui craps out - need to fix algo first


void DFSfindDepths(
    string node,
    ModulePositionMap & positions,
    const ConnectionsMap & connections,
    int depth,
    const string & terminator
)
{
    // depth-first version
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
    // breadth-first version
    // Note - Djikstra won't do here actually, 
    // as we _want_ to travel the graph in all directions
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


void fallDownX(
    ModulePositionMap & modulePositions,
    const ConnectionsMap & connections,
    set<string> & xFellTrough,
    set<pair<int, int>> & gridOccupation,
    const string & start,
    int iteration
) {
    string n = start;
    while (true) {
        if (!xFellTrough.count(n)) {
            auto position = make_pair(iteration, modulePositions[n].y);
            while (gridOccupation.count(position)) {
                // something is allready there, move in x momentarily
                iteration += 2; // odd even thing below ...
                position = make_pair(iteration, modulePositions[n].y);
            }
            gridOccupation.insert(position);
            // place symmetric around 0!
            modulePositions[n].x = (float)(((iteration % 2) == 0) ? (-iteration / 2) : (iteration / 2 + 1));
        }
        xFellTrough.insert(n);
        auto itn = connections.find(n);
        // check if we can continue (graph may be broken)
        if (itn == connections.end()) break;
        // find the next node which has the lowest y, 
        // while still larger or equal to current y
        // -> this favours the leg connection with most stuff on it, 
        //    but does not guarantee no overlap along that leg should
        //    there be split/join stuff along it
        auto nn = n;
        float min_y = FLT_MAX;
        for (const auto & c : itn->second) {
            auto n_y = modulePositions[n].y; 
            auto c_y = modulePositions[c].y;
            if ((min_y >= c_y)
            && (c_y < n_y) // need some check in the event of feedback loop
            )
            {
                min_y = c_y;
                nn = c;
            }
        }
        if (nn == n) break; // nowhere to go
        n = nn;
    }
    return;
}


void setNodePositionsInner(
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
            modulePositions[start].y = p.second.y - 1.f;
        }
    }

    // and shift everything to positive
    float y_bias = modulePositions[start].y;
    for (auto &p : modulePositions) {
        p.second.y -= y_bias;
    }


    // Let's do x

    // Remember, start now has y 0!

    set<string> xFellFrom; // track from where we started
    set<string> xFellTrough; // track where we passed through
    set<pair<int, int>> gridOccupation; // track where things are placed to mitigate overlap
    queue<pair<string, int>> xQueue; // node + x position to work over (as we do breadth-first)

    // for each node, starting from 'stop',
    // bubble upwards and set x.
    // queue all connections, first being zero
    // -- this does not yeild a global solution but is decent
    xQueue.push(make_pair(stop, 0));
    while (xQueue.size()) {
        auto p = xQueue.front(); xQueue.pop();
        auto m = p.first;
        if (xFellFrom.count(m)) continue;
        auto d = p.second;
        xFellFrom.insert(m);
        fallDownX(modulePositions, 
            undirectedConnections, 
            xFellTrough, 
            gridOccupation, 
            m, 
            d
        );
        auto cit = undirectedConnections.find(m);
        if (cit == undirectedConnections.end()) continue;
        int i = 0;
        for (const auto& c : cit->second) {
            xQueue.push(make_pair(c, d+i));
            i++;
        }
    }

    // and shift everything to positive
    float x_bias = FLT_MAX;
    for (auto &p : modulePositions) {
        if (p.second.x < x_bias) {
            x_bias = p.second.x;
        }
    }
    for (auto &p : modulePositions) {
        p.second.x -= x_bias;
    }
    modulePositions[start].x = modulePositions[stop].x;
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
        modulePositions[mv.name] = XY(initial_x, INT_MIN);
    }
    modulePositions[start] = XY(initial_x, INT_MIN);
    modulePositions[stop] = XY(initial_x, INT_MIN);

    // find all y positions and return the longest path
    setNodePositionsInner(
        modulePositions, 
        undirectedConnections, 
        backwardsConnections, 
        forwardsConnections, 
        start, 
        stop
    );
}
