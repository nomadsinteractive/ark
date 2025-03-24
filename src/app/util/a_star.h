#pragma once

#include <memory>

#include "core/types/optional.h"

#include "graphics/base/v3.h"

namespace ark {

template<typename T, typename H = std::hash<T>> class AStar {
private:
    struct SearchingRoute {
        float _score;
        float _weight;
        Vector<T> _route_path;

        bool operator < (const SearchingRoute& other) const {
            return _score < other._score;
        }
    };

public:
    AStar(T start, T goal)
        : _start(std::move(start)), _goal(std::move(goal)), _routes{ SearchingRoute{getHeuristicValue(_start), 0, {_start}} } {
    }

    Vector<T> findRoute() {
        while(_routes.size() > 0) {
            if(_routes.begin()->_route_path.back() == _goal)
                return _routes.begin()->_route_path;

            if(Optional<SearchingRoute> inflated = inflate())
                _routes.insert(std::move(inflated.value()));
            else
                _routes.erase(_routes.begin());
        }
        return {};
    }

private:
    Optional<SearchingRoute> inflate() {
        std::unique_ptr<T> nextNode;
        float nextWeight;
        float nextScore;
        float bestScore = std::numeric_limits<float>::max();
        const auto visitor = [this, &nextNode, &nextWeight, &nextScore, &bestScore] (T node, float weight) {
            if(_close_set.find(node) == _close_set.end()) {
                const SearchingRoute& next = *_routes.begin();
                nextWeight = next._weight + weight;
                nextScore = nextWeight + getHeuristicValue(node);
                if(bestScore > nextScore) {
                    bestScore = nextScore;
                    nextNode = std::make_unique<T>(std::move(node));
                }
            }
        };

        T(_routes.begin()->_route_path.back()).visitAdjacentNodes(visitor);
        if(nextNode) {
            SearchingRoute bestRoute{nextScore, nextWeight, _routes.begin()->_route_path};
            _close_set.insert(*nextNode);
            bestRoute._route_path.push_back(std::move(*nextNode));
            return bestRoute;
        }
        return Optional<SearchingRoute>();
    }

    float getHeuristicValue(T& from) const {
        const V3 d = from.position() - _goal.position();
        return d.dot(d);
    }

private:
    T _start, _goal;
    std::multiset<SearchingRoute> _routes;
    HashSet<T, H> _close_set;
};

}

