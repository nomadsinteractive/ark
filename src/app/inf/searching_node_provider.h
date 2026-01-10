#pragma once

#include <functional>

#include "graphics/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class SearchingNodeProvider {
public:
    virtual ~SearchingNodeProvider() = default;

//  [[script::bindings::interface]]
    virtual void onVisitAdjacentNodes(const V3& position, const std::function<void(SearchingNode, float)>& visitor) = 0;
//  [[script::bindings::interface]]
    virtual bool testGoalReached(const V3& position) = 0;
};

}