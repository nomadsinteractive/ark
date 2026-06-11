#pragma once

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API PathFinder {
public:
//  [[script::bindings::auto]]
    PathFinder(sp<SearchingNodeProvider> searchingNodeProvider);

//  [[script::bindings::auto]]
    Vector<V3> findPath(const V3i& startId, const V3& startPosition, const V3i& endId, const V3& endPosition, float distanceTolerance) const;

private:
    sp<SearchingNodeProvider> _searching_node_provider;
};

}
