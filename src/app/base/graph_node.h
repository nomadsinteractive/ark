#pragma once

#include <vector>

#include "core/collection/list.h"

#include "graphics/base/v3.h"

#include "core/types/shared_ptr.h"
#include "core/types/weak_ptr.h"

namespace ark {

class ARK_API GraphNode {
public:
    GraphNode(Graph& graph, const V3& position, Box tag);

//  [[script::bindings::property]]
    const V3& position() const;
//  [[script::bindings::auto]]
    void addRoute(GraphNode& toNode, float length);

//  [[script::bindings::property]]
    const Box& tag() const;
//  [[script::bindings::auto]]
    void setTag(Box tag);

//  [[script::bindings::auto]]
    std::vector<Box> findRoute(GraphNode& goal);

    const std::vector<GraphRoute>& inRoutes() const;
    std::vector<GraphRoute>& inRoutes();

    const std::vector<GraphRoute>& outRoutes() const;
    std::vector<GraphRoute>& outRoutes();

private:
    Graph& _graph;
    V3 _position;
    Box _tag;

    std::vector<GraphRoute> _in_routes;
    std::vector<GraphRoute> _out_routes;

};

}

