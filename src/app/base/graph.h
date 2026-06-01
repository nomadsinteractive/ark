#pragma once

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"
#include "app/inf/searching_node_provider.h"

namespace ark {

class ARK_API Graph : public SearchingNodeProvider {
public:
//  [[script::bindings::auto]]
    Graph();
    ~Graph();

//  [[script::bindings::property]]
    const Map<int32_t, sp<GraphNode>>& nodes() const;
//  [[script::bindings::auto]]
    sp<GraphNode> makeNode(const V3& position, Box tag = nullptr);
//  [[script::bindings::auto]]
    bool hasNode(const GraphNode& node) const;

    sp<GraphNode> toSharedPtr(const GraphNode& node) const;

    void onVisitAdjacentNodes(int32_t nodeId, const V3& position, const std::function<void(const SearchingNode&)>& visitor) override;

private:
    Map<int32_t, sp<GraphNode>> _nodes;
};

}

