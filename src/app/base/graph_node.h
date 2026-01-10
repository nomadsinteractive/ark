#pragma once

#include "core/types/box.h"

#include "graphics/base/v3.h"

#include "app/forwarding.h"
#include "app/base/graph.h"
#include "app/base/graph_route.h"
#include "app/inf/searching_node_provider.h"

namespace ark {

class ARK_API GraphNode final : public SearchingNodeProvider {
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
    Vector<V3> findPath(GraphNode& goal);

    sp<GraphNode> toSharedPtr() const;

    const Graph& graph() const;

    void detach();

    const Vector<GraphRoute>& inRoutes() const;
    Vector<GraphRoute>& inRoutes();

    const Vector<GraphRoute>& outRoutes() const;
    Vector<GraphRoute>& outRoutes();

    void onVisitAdjacentNodes(const V3& position, const std::function<void(SearchingNode, float)>& visitor) override;

private:
    Graph& _graph;
    V3 _position;
    Box _tag;

    Vector<GraphRoute> _in_routes;
    Vector<GraphRoute> _out_routes;
};

}

