#include "app/base/a_star.h"

#include <unordered_set>

#include "core/util/strings.h"

#include "app/base/graph_node.h"
#include "app/base/graph_route.h"


namespace ark {

namespace {

struct SearchingRoute {
    float _score;
    std::vector<GraphNode*> _route_path;
};

class SearchingGraph {
public:

    std::vector<GraphNode*> astar(GraphNode& start, GraphNode& goal) {
        _goal = &goal;

        std::vector<SearchingRoute> routes;
        std::vector<SearchingRoute> newRoutes;
        routes.push_back(SearchingRoute{getHeuristicValue(start), {&start}});
        while(routes.front()._route_path.back() != _goal) {

            for(SearchingRoute& i : routes)
                inflate(i, newRoutes);

            if(newRoutes.empty()) {
                const SearchingRoute* bestRoute = findBestSearchingRoute(routes);
                return bestRoute ? bestRoute->_route_path : std::vector<GraphNode*>{};
            }

            routes = std::move(newRoutes);
        }
        return routes.front()._route_path;
    }

private:
    static bool _routingCompare(const SearchingRoute& a, const SearchingRoute& b) {
        return a._score < b._score;
    }

    void inflate(SearchingRoute& route, std::vector<SearchingRoute>& newRoutes) {
        for(GraphRoute& i : route._route_path.back()->outRoutes()) {
            GraphNode& node = i.exit();
            if(_close_set.find(&node) == _close_set.end()) {
                _close_set.insert(&node);
                SearchingRoute next = route;
                next._score += i.length();
                next._score += getHeuristicValue(node);
                next._route_path.push_back(&node);
                newRoutes.insert(std::lower_bound(newRoutes.begin(), newRoutes.end(), next, _routingCompare), std::move(next));
            }
        }
    }

    float getHeuristicValue(GraphNode& from) {
        const V3 d = from.position() - _goal->position();
        return d.dot(d);
    }

    const SearchingRoute* findBestSearchingRoute(const std::vector<SearchingRoute>& routes) const {
        const SearchingRoute* bestRoute = nullptr;
        for(const SearchingRoute& i : routes)
            if(i._route_path.back() == _goal && (bestRoute == nullptr || bestRoute->_score > i._score))
                bestRoute = &i;
        return bestRoute;
    }

private:
    GraphNode* _goal;
    std::unordered_set<GraphNode*> _close_set;
};

}

std::vector<GraphNode*> astar(GraphNode& start, GraphNode& goal)
{
    SearchingGraph searchingGraph;
    return searchingGraph.astar(start, goal);
}

}
