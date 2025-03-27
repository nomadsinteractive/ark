#pragma once

#include <memory>

#include "core/types/optional.h"

#include "graphics/base/v3.h"

namespace ark {

template<typename T> class AStar {
private:
    struct SearchingPath {
        float _score;
        float _weight;
        Vector<T> _nodes;

        bool operator < (const SearchingPath& other) const {
            return _score < other._score;
        }
    };

public:
    AStar(T start, T goal)
        : _start(std::move(start)), _goal(std::move(goal)), _paths{ SearchingPath{getHeuristicValue(_start), 0, {_start}} } {
    }

    Vector<T> findPath() {
        while(_paths.size() > 0) {
            if(_paths.begin()->_nodes.back() == _goal)
                return _paths.begin()->_nodes;

            if(Optional<SearchingPath> inflated = inflate())
                _paths.insert(std::move(inflated.value()));
            else
                _paths.erase(_paths.begin());
        }
        return {};
    }

private:
    Optional<SearchingPath> inflate() {
        std::unique_ptr<T> nextNode;
        float nextWeight;
        float nextScore;
        float bestScore = std::numeric_limits<float>::max();
        const auto visitor = [this, &nextNode, &nextWeight, &nextScore, &bestScore] (T node, float weight) {
            if(_close_set.find(node) == _close_set.end()) {
                const SearchingPath& next = *_paths.begin();
                nextWeight = next._weight + weight;
                nextScore = nextWeight + getHeuristicValue(node);
                if(bestScore > nextScore) {
                    bestScore = nextScore;
                    nextNode = std::make_unique<T>(std::move(node));
                }
            }
        };

        T(_paths.begin()->_nodes.back()).visitAdjacentNodes(visitor);
        if(nextNode) {
            SearchingPath bestRoute{nextScore, nextWeight, _paths.begin()->_nodes};
            _close_set.insert(*nextNode);
            bestRoute._nodes.push_back(std::move(*nextNode));
            return bestRoute;
        }
        return Optional<SearchingPath>();
    }

    float getHeuristicValue(T& from) const {
        const V3 d = from.position() - _goal.position();
        return d.hypot();
    }

private:
    T _start, _goal;
    std::multiset<SearchingPath> _paths;
    Set<T> _close_set;
};

}

