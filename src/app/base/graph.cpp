#include "app/base/graph.h"

#include "app/base/graph_node.h"
#include "app/base/graph_route.h"

namespace ark {

static bool _graph_node_compare(const GraphNode& n1, const GraphNode& n2)
{
    return &n1 < &n2;
}

Graph::Graph()
{
}

Graph::~Graph()
{
    std::vector<std::pair<GraphNode&, GraphNode&>> externalRoutes;

    for(GraphNode& i : _nodes)
    {
        for(GraphRoute& j : i.inRoutes())
            if(j.isExternal())
                externalRoutes.emplace_back(j.entry(), j.exit());

        for(GraphRoute& j : i.outRoutes())
            if(j.isExternal())
                externalRoutes.emplace_back(j.entry(), j.exit());
    }

    for(const auto [i, j] : externalRoutes)
        i.disconnect(j);
}

const std::vector<sp<GraphNode>>& Graph::nodes() const
{
    return _nodes;
}

sp<GraphNode> Graph::makeNode(const V3& position, Box tag)
{
    sp<GraphNode> node = sp<GraphNode>::make(*this, position, std::move(tag));
    _nodes.insert(std::lower_bound(_nodes.begin(), _nodes.end(), node), node);
    return node;
}

bool Graph::hasNode(const GraphNode& node) const
{
    const auto iter = std::lower_bound(_nodes.begin(), _nodes.end(), node, _graph_node_compare);
    return iter != _nodes.end() && iter->get() == &node;
}

sp<GraphNode> Graph::toSharedPtr(const GraphNode& node) const
{
    const auto iter = std::lower_bound(_nodes.begin(), _nodes.end(), node, _graph_node_compare);
    ASSERT(iter != _nodes.end() && iter->get() == &node);
    return *iter;
}

}
