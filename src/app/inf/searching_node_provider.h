#pragma once

#include <functional>

#include "graphics/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class SearchingNodeProvider {
public:
    virtual ~SearchingNodeProvider() = default;

//  [[script::bindings::interface]]
    virtual void onVisitAdjacentNodes(int32_t nodeId, const V3& position, const std::function<bool(const SearchingNode&)>& visitor) = 0;
};

}