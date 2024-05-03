#include "app/util/convex_hull_resolver.h"

#include "app/util/quickhull.h"

namespace ark {

template<typename T, size_t N> std::deque<std::array<T, N>> resolveConvexHull(const std::deque<std::array<T, N>>& points, bool checkResult) {
    typedef typename std::deque<std::array<T, N>>::const_iterator PointIterator;
    typedef quick_hull<PointIterator, T> QuickHull;
    const auto eps = std::numeric_limits<T>::epsilon();
    QuickHull qh{N, eps};
    qh.add_points(std::cbegin(points), std::cend(points));
    auto initial_simplex = qh.get_affine_basis();
    ASSERT(initial_simplex.size() >= N + 1);
    qh.create_initial_simplex(std::cbegin(initial_simplex), std::prev(std::cend(initial_simplex)));
    qh.create_convex_hull();

    if(checkResult)
        ASSERT(qh.check());

    ASSERT(qh.facets_.size() > 0);
    std::map<std::array<T, N>, PointIterator> facetMap;
    for(const typename QuickHull::facet& i : qh.facets_) {
        ASSERT(i.vertices_.size() == 2);
        facetMap[*i.vertices_.at(0)] = i.vertices_.at(1);
    }

    std::deque<std::array<T, N>> convexHull;
    std::array<T, N> hullPoint = *qh.facets_.at(0).vertices_.at(0);
    for(size_t i = 0; i < qh.facets_.size(); ++i) {
        convexHull.push_back(hullPoint);
        const auto iter = facetMap.find(hullPoint);
        ASSERT(iter != facetMap.end());
        hullPoint = *(iter->second);
    }
    return convexHull;
}

ConvexHullResolver::ConvexHullResolver(bool checkError)
    : _check_error(checkError)
{
}

std::deque<std::array<int32_t, 2>> ConvexHullResolver::resolve(const std::deque<std::array<int32_t, 2>>& points)
{
    return resolveConvexHull(points, _check_error);
}

std::deque<std::array<float, 2>> ConvexHullResolver::resolve(const std::deque<std::array<float, 2>>& points)
{
    return resolveConvexHull(points, _check_error);
}

}
