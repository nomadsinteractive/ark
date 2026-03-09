#include "app/base/searching_node.h"

#include "core/util/math.h"

#include "app/base/graph_node.h"

namespace ark {

SearchingNode::SearchingNode(const sp<GraphNode>& graphNode)
    : _searching_node_provider(*graphNode), _position(graphNode->position())
{
}

SearchingNode::SearchingNode(SearchingNodeProvider& searchingNodeProvider, const V3& position)
    : _searching_node_provider(searchingNodeProvider), _position(position)
{
}

const V3& SearchingNode::position() const
{
    return _position;
}

void SearchingNode::visitAdjacentNodes(const std::function<void(SearchingNode, float)>& visitor) const
{
    _searching_node_provider.onVisitAdjacentNodes(_position, visitor);
}

bool SearchingNode::operator<(const SearchingNode& other) const
{
    if(_position.x() != other._position.x())
        return _position.x() < other._position.x();
    if(_position.y() != other._position.y())
        return _position.y() < other._position.y();
    return _position.z() < other._position.z();
}

}
