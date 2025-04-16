#pragma once

#include <functional>

#include "graphics/forwarding.h"

#include "app/forwarding.h"

namespace ark {

//[[script::bindings::class]]
class SearchingNodeProvider {
public:
    virtual ~SearchingNodeProvider() = default;

    virtual void onVisitAdjacentNodes(const V3& position, const std::function<void(SearchingNode, float)>& visitor) = 0;
};

}