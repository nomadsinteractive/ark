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

//  [[script::bindings::property]]
    const std::set<sp<GraphNode>>& nodes() const;
//  [[script::bindings::auto]]
    sp<GraphNode> makeNode(const V3& position, Box tag = nullptr);

private:
    std::set<sp<GraphNode>> _nodes;
};

}

