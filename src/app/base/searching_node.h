#pragma once

#include <functional>

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

#include "app/forwarding.h"

namespace ark {

class SearchingNode {
public:
    SearchingNode(const sp<GraphNode>& graphNode);
    SearchingNode(SearchingNodeProvider& searchingNodeProvider, const V3& position);

    const V3& position() const;

    void visitAdjacentNodes(const std::function<void(SearchingNode, float)>& visitor) const;

    bool operator < (const SearchingNode& other) const;
    bool operator == (const SearchingNode& other) const;
    bool operator != (const SearchingNode& other) const;

private:
    SearchingNodeProvider& _searching_node_provider;
    V3 _position;
};

}
