#pragma once

#include <set>

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API Graph {
public:
//  [[script::bindings::auto]]
    Graph();
    ~Graph();

//  [[script::bindings::property]]
    const std::vector<sp<GraphNode>>& nodes() const;
//  [[script::bindings::auto]]
    sp<GraphNode> makeNode(const V3& position, Box tag = nullptr);

//  [[script::bindings::auto]]
    bool hasNode(const GraphNode& node) const;

    sp<GraphNode> toSharedPtr(const GraphNode& node) const;

private:
    std::vector<sp<GraphNode>> _nodes;
};

}

