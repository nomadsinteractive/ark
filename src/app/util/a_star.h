#pragma once

#include <memory>

#include "core/types/optional.h"
#include "core/util/math.h"

#include "graphics/base/v3.h"

#include "app/base/searching_node.h"
#include "app/inf/searching_node_provider.h"

namespace ark {

class AStar {
private:
    struct SearchingPath {
        float _score;
        float _weight;
        Vector<SearchingNode> _nodes;

        bool operator < (const SearchingPath& other) const {
            return _score < other._score;
        }
    };

public:
    AStar(SearchingNode start, SearchingNode goal)
        : _start(std::move(start)), _goal(std::move(goal)), _paths{ SearchingPath{std::sqrt(getHeuristicValue2(_start)), 0, {_start}} } {
    }

    Vector<SearchingNode> findPath(SearchingNodeProvider& searchingNodeProvider, const float distanceTolerance) {
        const float dt2 = distanceTolerance * distanceTolerance;
        while(_paths.size() > 0) {
            if(const SearchingNode& backNode = _paths.begin()->_nodes.back(); backNode._is_goal) {
                if(backNode._is_goal.value())
                    return _paths.begin()->_nodes;
            } else if(getHeuristicValue2(backNode) <= dt2)
                return _paths.begin()->_nodes;

            if(Optional<SearchingPath> inflated = inflate(searchingNodeProvider))
                _paths.insert(std::move(inflated.value()));
            else
                _paths.erase(_paths.begin());
        }
        return {};
    }

private:
    Optional<SearchingPath> inflate(SearchingNodeProvider& searchingNodeProvider) {
        std::optional<SearchingNode> nextNode;
        float nextWeight;
        float nextScore;
        float bestScore = std::numeric_limits<float>::max();
        const auto visitor = [this, &nextNode, &nextWeight, &nextScore, &bestScore] (const SearchingNode& node) {
            if(!_close_set.contains(node)) {
                const SearchingPath& next = *_paths.begin();
                const float nodeWeight = node._weight ? node._weight.value() : Math::distance(node._position, next._nodes.back()._position);
                nextWeight = next._weight + nodeWeight;
                nextScore = nextWeight + std::sqrt(getHeuristicValue2(node));
                if(bestScore > nextScore) {
                    bestScore = nextScore;
                    nextNode = {node};
                }
            }
        };

        searchingNodeProvider.onVisitAdjacentNodes(_paths.begin()->_nodes.back()._id, _paths.begin()->_nodes.back()._position, visitor);
        if(nextNode) {
            SearchingPath bestRoute = {nextScore, nextWeight, _paths.begin()->_nodes};
            _close_set.insert(nextNode.value());
            bestRoute._nodes.push_back(nextNode.value());
            return bestRoute;
        }

        return {};
    }

    float getHeuristicValue2(const SearchingNode& from) const {
        const V3 d = from._position - _goal._position;
        return Math::hypot2(d);
    }

private:
    SearchingNode _start, _goal;
    std::multiset<SearchingPath> _paths;
    Set<SearchingNode> _close_set;
};

}

