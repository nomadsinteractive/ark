#include "app/base/graph_node.h"

#include "app/base/graph.h"
#include "app/base/graph_route.h"
#include "app/base/searching_node.h"
#include "app/util/a_star.h"

namespace ark {

namespace {

void disconnectNode(Vector<GraphRoute>& routes, GraphNode* toNode, const bool entryOrExit)
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

GraphNode::GraphNode(Graph& graph, const V3& position, Box tag)
    : _graph(graph), _position(position), _tag(std::move(tag))
{
}

const V3& GraphNode::position() const
{
    return _position;
}

void GraphNode::addRoute(GraphNode& toNode, const float length, const bool bidirectional)
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
    disconnectNode(_in_routes, &toNode, true);
    disconnectNode(_out_routes, &toNode, false);
    disconnectNode(toNode._in_routes, this, true);
    disconnectNode(toNode._out_routes, this, false);
}

const Box& GraphNode::tag() const
{
    return _tag;
}

void GraphNode::setTag(Box tag)
{
    _tag = std::move(tag);
}

Vector<V3> GraphNode::findPath(GraphNode& goal)
{
    Vector<V3> result;
    AStar<SearchingNode> pathFinder(this->toSharedPtr(), goal.toSharedPtr());
    for(const SearchingNode& i : pathFinder.findPath())
        result.push_back(i.position());
    return result;
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

const Vector<GraphRoute>& GraphNode::inRoutes() const
{
    return _in_routes;
}

Vector<GraphRoute>& GraphNode::inRoutes()
{
    return _in_routes;
}

const Vector<GraphRoute>& GraphNode::outRoutes() const
{
    return _out_routes;
}

Vector<GraphRoute>& GraphNode::outRoutes()
{
    return _out_routes;
}

void GraphNode::onVisitAdjacentNodes(const V3& /*position*/, const std::function<void(SearchingNode, float)>& visitor)
{
    for(GraphRoute& i : _out_routes)
        visitor(SearchingNode(i.exit().toSharedPtr()), i.weight());
}

bool GraphNode::testGoalReached(const V3& position)
{
    return false;
}

}
