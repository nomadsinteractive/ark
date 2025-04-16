#pragma once

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
    const Vector<sp<GraphNode>>& nodes() const;
//  [[script::bindings::auto]]
    sp<GraphNode> makeNode(const V3& position, Box tag = nullptr);
//  [[script::bindings::auto]]
    bool hasNode(const GraphNode& node) const;

    sp<GraphNode> toSharedPtr(const GraphNode& node) const;

private:
    Vector<sp<GraphNode>> _nodes;
};

}

