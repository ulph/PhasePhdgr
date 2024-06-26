#pragma once

#include <string>
#include <map>
#include <set>
#include "GraphEditorModel.hpp"


using namespace std;

typedef map<string, set<string>> ConnectionsMap;
typedef map<string, XY> ModulePositionMap;

void setNodePositions(
    const ConnectionGraphDescriptor& connectionGraphDescriptor,
    ModulePositionMap & modulePositions,
    const string &start,
    const string &stop
);
