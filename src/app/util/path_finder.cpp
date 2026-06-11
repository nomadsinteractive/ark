#include "app/util/path_finder.h"

#include "core/util/math.h"

#include "graphics/base/v3.h"

#include "app/base/searching_node.h"
#include "app/inf/searching_node_provider.h"
#include "app/util/a_star.h"

namespace ark {

PathFinder::PathFinder(sp<SearchingNodeProvider> searchingNodeProvider)
    : _searching_node_provider(std::move(searchingNodeProvider))
{
}

Vector<V3> PathFinder::findPath(const V3i& startId, const V3& startPosition, const V3i& endId, const V3& endPosition, const float distanceTolerance) const
{
    Vector<V3> result;
    AStar pathFinder({startId, startPosition}, {endId, endPosition, true});
    for(const SearchingNode& i : pathFinder.findPath(*_searching_node_provider, distanceTolerance))
        result.push_back(i._position);
    return result;
}

}
