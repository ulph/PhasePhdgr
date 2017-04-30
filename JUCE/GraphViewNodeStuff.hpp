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
    int currentDepth = positions[node].y;
    if (depth <= currentDepth) return;
    positions[node].y = depth;
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
    int y_bias = modulePositions[start].y;
    for (auto &p : modulePositions) {
        p.second.y -= y_bias;
    }
}
