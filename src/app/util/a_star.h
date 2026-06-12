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
    AStar(SearchingNode start, SearchingNode goal);

    Vector<SearchingNode> findPath(SearchingNodeProvider& searchingNodeProvider, float distanceTolerance);

private:
    void expand(SearchingNodeProvider& searchingNodeProvider, const SearchingNode& node);
    bool isGoal(const SearchingNode& node, float dt2) const;
    Vector<SearchingNode> reconstructPath(const SearchingNode& goal) const;
    float getHeuristicValue2(const SearchingNode& from) const;

private:
    SearchingNode _goal;
    std::priority_queue<OpenNode, Vector<OpenNode>, CompareOpenNode> _open;
    Set<V3i> _closed;
    Map<V3i, float> _g_score;
    Map<V3i, SearchingNode> _came_from;
};

}
