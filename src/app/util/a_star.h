#pragma once

#include <algorithm>
#include <memory>
#include <queue>
#include <vector>

#include "core/types/optional.h"
#include "core/util/math.h"

#include "graphics/base/v3.h"

#include "app/base/searching_node.h"
#include "app/inf/searching_node_provider.h"

namespace ark {

class AStar {
private:
    struct OpenNode {
        float _score;
        SearchingNode _node;
    };

    //  Orders the priority queue as a min-heap on '_score' so that top() yields the most promising
    //  (lowest f-score) node.
    struct CompareOpenNode {
        bool operator() (const OpenNode& lhs, const OpenNode& rhs) const {
            return lhs._score > rhs._score;
        }
    };

public:
    AStar(SearchingNode start, SearchingNode goal)
        : _goal(std::move(goal)) {
        _g_score[start._id] = 0;
        _open.push({std::sqrt(getHeuristicValue2(start)), std::move(start)});
    }

    Vector<SearchingNode> findPath(SearchingNodeProvider& searchingNodeProvider, const float distanceTolerance) {
        const float dt2 = distanceTolerance * distanceTolerance;
        while(!_open.empty()) {
            const OpenNode current = _open.top();
            _open.pop();

            //  The open set keeps stale duplicates (lazy deletion): a node is re-queued whenever a
            //  cheaper route to it is found, so an entry may refer to a node that was already
            //  finalized through a better path. Closing it on pop discards those stale copies and
            //  guarantees each node is expanded exactly once.
            if(!_closed.insert(current._node._id).second)
                continue;

            const SearchingNode& node = current._node;
            if(isGoal(node, dt2))
                return reconstructPath(node);

            expand(searchingNodeProvider, node);
        }
        return {};
    }

private:
    void expand(SearchingNodeProvider& searchingNodeProvider, const SearchingNode& node) {
        const float gScore = _g_score.at(node._id);
        const auto visitor = [this, &node, gScore] (const SearchingNode& adjacent) -> bool {
            if(_closed.contains(adjacent._id))
                return false;

            const float edgeWeight = adjacent._weight ? adjacent._weight.value() : Math::distance(adjacent._position, node._position);
            const float tentativeGScore = gScore + edgeWeight;
            if(const auto iter = _g_score.find(adjacent._id); iter == _g_score.end() || tentativeGScore < iter->second) {
                _g_score[adjacent._id] = tentativeGScore;
                _came_from[adjacent._id] = node;
                _open.push({tentativeGScore + std::sqrt(getHeuristicValue2(adjacent)), adjacent});
            }
            return true;
        };
        searchingNodeProvider.onVisitAdjacentNodes(node._id, node._position, visitor);
    }

    //  Three-state goal test: a provider that sets '_is_goal' has the final say (true terminates the
    //  search, false explicitly rejects the node and prevents the tolerance fallback); when it is
    //  unset we fall back to the straight-line distance tolerance.
    bool isGoal(const SearchingNode& node, const float dt2) const {
        if(node._is_goal)
            return node._is_goal.value();
        return getHeuristicValue2(node) <= dt2;
    }

    Vector<SearchingNode> reconstructPath(const SearchingNode& goal) const {
        Vector<SearchingNode> path = {goal};
        for(auto iter = _came_from.find(goal._id); iter != _came_from.end(); iter = _came_from.find(iter->second._id))
            path.push_back(iter->second);
        std::reverse(path.begin(), path.end());
        return path;
    }

    float getHeuristicValue2(const SearchingNode& from) const {
        const V3 d = from._position - _goal._position;
        return Math::hypot2(d);
    }

private:
    SearchingNode _goal;
    std::priority_queue<OpenNode, Vector<OpenNode>, CompareOpenNode> _open;
    Set<V3i> _closed;
    Map<V3i, float> _g_score;
    Map<V3i, SearchingNode> _came_from;
};

}
