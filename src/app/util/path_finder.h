#pragma once

#include "core/base/api.h"

#include "app/base/graph.h"

namespace ark {

class ARK_API PathFinder {
public:
//  [[script::bindings::auto]]
    PathFinder(Behavior& behavior, StringView visitAdjacentNodes = "visit_adjacent_nodes");

//  [[script::bindings::auto]]
    Vector<V3> findPath(const V3& start, const V3& end);
//  [[script::bindings::auto]]
    sp<GraphSearchingNode> makeGraphSearchingNode(const V3& position);

private:
    sp<Graph::SearchingNodeProvider> _searching_node_provider;
};

}
