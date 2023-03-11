#pragma once

#include <vector>

#include "app/forwarding.h"

namespace ark {

std::vector<GraphNode*> astar(GraphNode& start, GraphNode& goal);

}

