#include "app/base/graph_node.h"

#include "app/base/graph_route.h"
#include "app/base/a_star.h"

namespace ark {

GraphNode::GraphNode(Graph& graph, const V3& position, Box tag)
    : _graph(graph), _position(position), _tag(std::move(tag))
{
}

const V3& GraphNode::position() const
{
    return _position;
}

void GraphNode::addRoute(GraphNode& toNode, float length)
{
    _out_routes.emplace_back(*this, toNode, length);
    toNode.inRoutes().emplace_back(*this, toNode, length);
}

const Box& GraphNode::tag() const
{
    return _tag;
}

void GraphNode::setTag(Box tag)
{
    _tag = std::move(tag);
}

std::vector<Box> GraphNode::findRoute(GraphNode& goal)
{
    std::vector<Box> routes;
    for(GraphNode* i : astar(*this, goal))
        routes.push_back(i->tag());
    return routes;
}

const std::vector<GraphRoute>& GraphNode::inRoutes() const
{
    return _in_routes;
}

std::vector<GraphRoute>& GraphNode::inRoutes()
{
    return _in_routes;
}

const std::vector<GraphRoute>& GraphNode::outRoutes() const
{
    return _out_routes;
}

std::vector<GraphRoute>& GraphNode::outRoutes()
{
    return _out_routes;
}

}
