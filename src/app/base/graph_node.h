#pragma once

#include <vector>

#include "core/collection/list.h"

#include "graphics/base/v3.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API GraphNode {
public:
    GraphNode(Graph& graph, const V3& position, Box tag);

//  [[script::bindings::property]]
    const V3& position() const;
//  [[script::bindings::auto]]
    void addRoute(GraphNode& toNode, float length, bool bidirectional = true);
//  [[script::bindings::auto]]
    void disconnect(GraphNode& toNode);

//  [[script::bindings::property]]
    const Box& tag() const;
//  [[script::bindings::auto]]
    void setTag(Box tag);

//  [[script::bindings::auto]]
    std::vector<sp<GraphNode>> findRoute(GraphNode& goal);

    sp<GraphNode> toSharedPtr() const;

    const Graph& graph() const;

    void detach();

    const std::vector<GraphRoute>& inRoutes() const;
    std::vector<GraphRoute>& inRoutes();

    const std::vector<GraphRoute>& outRoutes() const;
    std::vector<GraphRoute>& outRoutes();

private:
    void doDisconnect(std::vector<GraphRoute>& routes, GraphNode* toNode, bool entryOrExit);


private:
    Graph& _graph;
    V3 _position;
    Box _tag;

    std::vector<GraphRoute> _in_routes;
    std::vector<GraphRoute> _out_routes;
};

}

