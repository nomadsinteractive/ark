#include "app/base/graph_node.h"

#include "app/base/graph.h"
#include "app/base/graph_route.h"
#include "app/util/a_star.h"

namespace ark {

namespace {

class SearchingGraphNode {
public:
    SearchingGraphNode(GraphNode& graphNode)
        : _graph_node(graphNode) {
    }

    const V3& position() const {
        return _graph_node.position();
    }

    void visitAdjacentNodes(const std::function<void(SearchingGraphNode, float)>& visitor) {
        for(GraphRoute& i : _graph_node.outRoutes())
            visitor(i.exit(), i.weight());
    }

    bool operator == (const SearchingGraphNode& other) const {
        return &_graph_node == &other._graph_node;
    }

    bool operator != (const SearchingGraphNode& other) const {
        return &_graph_node != &other._graph_node;
    }

    GraphNode& _graph_node;
};

}

}

namespace std {

template <> struct hash<ark::SearchingGraphNode> {
    size_t operator()(const ark::SearchingGraphNode& str) const {
        return reinterpret_cast<size_t>(&str._graph_node);
    }
};

}

namespace ark {

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

Vector<sp<GraphNode>> GraphNode::findRoute(GraphNode& goal)
{
    Vector<sp<GraphNode>> result;
    AStar<SearchingGraphNode> searchingGraph(*this, goal);
    for(const SearchingGraphNode& i : searchingGraph.findRoute())
        result.push_back(i._graph_node.toSharedPtr());
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

void GraphNode::doDisconnect(Vector<GraphRoute>& routes, GraphNode* toNode, const bool entryOrExit)
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
