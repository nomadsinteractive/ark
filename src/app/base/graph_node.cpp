#include "app/base/graph_node.h"

#include "app/base/graph.h"
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

void GraphNode::addRoute(GraphNode& toNode, float length, bool bidirectional)
{
    _out_routes.emplace_back(*this, toNode, length);
    toNode.inRoutes().emplace_back(*this, toNode, length);
    if(bidirectional)
    {
        _in_routes.emplace_back(toNode, *this, length);
        toNode.outRoutes().emplace_back(toNode, *this, length);
    }
}

void GraphNode::disconnect(GraphNode& toNode)
{
    doDisconnect(_in_routes, &toNode, true);
    doDisconnect(_out_routes, &toNode, false);
    doDisconnect(toNode._in_routes, this, true);
    doDisconnect(toNode._out_routes, this, false);
}

const Box& GraphNode::tag() const
{
    return _tag;
}

void GraphNode::setTag(Box tag)
{
    _tag = std::move(tag);
}

std::vector<sp<GraphNode> > GraphNode::findRoute(GraphNode& goal)
{
    std::vector<sp<GraphNode>> routes;
    for(GraphNode* i : astar(*this, goal))
        routes.push_back(i->toSharedPtr());
    return routes;
}

sp<GraphNode> GraphNode::toSharedPtr() const
{
    return _graph.toSharedPtr(*this);
}

const Graph& GraphNode::graph() const
{
    return _graph;
}

void GraphNode::detach()
{
    _in_routes.clear();
    _out_routes.clear();
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

void GraphNode::doDisconnect(std::vector<GraphRoute>& routes, GraphNode* toNode, bool entryOrExit)
{
    for(auto iter = routes.begin(); iter != routes.end(); )
    {
        if((entryOrExit ? &iter->entry() : &iter->exit()) == toNode)
            iter = routes.erase(iter);
        else
            ++iter;
    }
}

}
