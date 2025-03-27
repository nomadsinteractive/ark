#pragma once

#include <functional>

#include "graph.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

#include "app/forwarding.h"

namespace ark {

class GraphSearchingNode {
public:
    GraphSearchingNode(sp<GraphNode> graphNode);
    GraphSearchingNode(const V3& position, sp<Graph::SearchingNodeProvider> searchingNodeProvider);

    const V3& position() const;

    void visitAdjacentNodes(const std::function<void(GraphSearchingNode, float)>& visitor) const;

    bool operator < (const GraphSearchingNode& other) const;
    bool operator == (const GraphSearchingNode& other) const;
    bool operator != (const GraphSearchingNode& other) const;

private:
    V3 _position;
    sp<Graph::SearchingNodeProvider> _searching_node_provider;
};

}
