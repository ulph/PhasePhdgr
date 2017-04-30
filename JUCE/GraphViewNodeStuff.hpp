#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include "GraphViewGfx.hpp"
#include "design.hpp"

using namespace std;

typedef map<string, unordered_set<string>> ConnectionsMap;
typedef map<string, XY> ModulePositionMap;

void updateNodesY(
    const string & node,
    ModulePositionMap & positions,
    const ConnectionsMap & unirectedConnections,
    int depth,
    const std::string & terminator
);

void setNodeY(
    ModulePositionMap & modulePositions,
    const ConnectionsMap & unirectedConnections,
    const string & start,
    const string & stop
);

void setNodeX(
    ModulePositionMap & modulePositions,
    const ConnectionsMap & backwardsConnections,
    const string & start,
    const string & stop
);

void setNodePositions(
    const ConnectionGraphDescriptor& connectionGraphDescriptor,
    ModulePositionMap & modulePositions,
    const string &start,
    const string &stop
);