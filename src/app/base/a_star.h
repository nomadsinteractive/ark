#pragma once

#include <unordered_set>
#include <vector>

#include "graphics/base/v3.h"

namespace ark {

template<typename T> class AStar {
private:
    struct SearchingRoute {
        float _score;
        std::vector<T> _route_path;
    };

public:
    AStar(T start, T goal)
        : _start(std::move(start)), _goal(std::move(goal)) {
    }

    std::vector<T> findRoute() {
        std::vector<SearchingRoute> routes;
        std::vector<SearchingRoute> newRoutes;
        routes.push_back(SearchingRoute{getHeuristicValue(_start), {_start}});
        while(routes.front()._route_path.back() != _goal) {

            for(SearchingRoute& i : routes)
                inflate(i, newRoutes);

            if(newRoutes.empty()) {
                const SearchingRoute* bestRoute = findBestSearchingRoute(routes);
                return bestRoute ? bestRoute->_route_path : std::vector<T>{};
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
        const auto visitor = [this, &route, &newRoutes] (T node, float weight) {
            if(_close_set.find(node) == _close_set.end()) {
                _close_set.insert(node);
                SearchingRoute next = route;
                next._score += weight;
                next._score += getHeuristicValue(node);
                next._route_path.push_back(node);
                newRoutes.insert(std::lower_bound(newRoutes.begin(), newRoutes.end(), next, _routingCompare), std::move(next));
            }
        };

        route._route_path.back().visitAdjacentNodes(visitor);
    }

    float getHeuristicValue(T& from) {
        const V3 d = from.position() - _goal.position();
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
    T _start, _goal;
    std::unordered_set<T> _close_set;
};

}

