#include "app/base/graph.h"

#include "app/base/graph_node.h"
#include "app/base/graph_route.h"

namespace ark {

Graph::Graph()
{
}

const std::set<sp<GraphNode>>& Graph::nodes() const
{
    return _nodes;
}

sp<GraphNode> Graph::makeNode(const V3& position, Box tag)
{
    sp<GraphNode> node = sp<GraphNode>::make(*this, position, std::move(tag));
    _nodes.insert(node);
    return node;
}

}
