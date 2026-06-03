#include "app/base/graph.h"

#include <ranges>

#include "app/base/graph_node.h"
#include "app/base/graph_route.h"
#include "app/base/searching_node.h"

namespace ark {

Graph::Graph()
{
}

Graph::~Graph()
{
    Vector<std::pair<GraphNode&, GraphNode&>> externalRoutes;

    for(const auto& i : _nodes | std::views::values)
    {
        for(GraphRoute& j : i->inRoutes())
            if(j.isExternal())
                externalRoutes.emplace_back(j.entry(), j.exit());

        for(GraphRoute& j : i->outRoutes())
            if(j.isExternal())
                externalRoutes.emplace_back(j.entry(), j.exit());
    }

    for(const auto [i, j] : externalRoutes)
        i.disconnect(j);
}

const Map<int32_t, sp<GraphNode>>& Graph::nodes() const
{
    return _nodes;
}

sp<GraphNode> Graph::makeNode(const V3& position, Box tag)
{
    const int32_t id = _nodes.empty() ? 0 : _nodes.rbegin()->second->id() + 1;
    sp<GraphNode> node = sp<GraphNode>::make(*this, id, position, std::move(tag));
    _nodes.insert({id, node});
    return node;
}

bool Graph::hasNode(const GraphNode& node) const
{
    const auto iter = _nodes.find(node.id());
    return iter != _nodes.end();
}

sp<GraphNode> Graph::toSharedPtr(const GraphNode& node) const
{
    const auto iter = _nodes.find(node.id());
    ASSERT(iter != _nodes.end() && iter->second.get() == &node);
    return iter->second;
}

void Graph::onVisitAdjacentNodes(const int32_t nodeId, const V3& position, const std::function<bool(const SearchingNode&)>& visitor)
{
    const sp<GraphNode>& node = _nodes.at(nodeId);
    for(GraphRoute& i : node->_out_routes)
        visitor({i.exit().id(), i.exit().position(), {}, i.weight()});
}

}
