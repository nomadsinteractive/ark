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

Vector<V3> PathFinder::findPath(const V3& start, const V3& end) const
{
    Vector<V3> result;
    AStar<SearchingNode> pathFinder({*_searching_node_provider, start}, {*_searching_node_provider, end});
    for(const SearchingNode& i : pathFinder.findPath())
        result.push_back(i.position());
    return result;
}

}
