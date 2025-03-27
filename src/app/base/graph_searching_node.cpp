#include "app/base/graph_searching_node.h"

#include "app/base/graph_node.h"
#include "core/util/math.h"

namespace ark {

GraphSearchingNode::GraphSearchingNode(sp<GraphNode> graphNode)
    : _position(graphNode->position()), _searching_node_provider(graphNode)
{
}

GraphSearchingNode::GraphSearchingNode(const V3& position, sp<Graph::SearchingNodeProvider> searchingNodeProvider)
    : _position(position), _searching_node_provider(std::move(searchingNodeProvider))
{
}

const V3& GraphSearchingNode::position() const
{
    return _position;
}

void GraphSearchingNode::visitAdjacentNodes(const std::function<void(GraphSearchingNode, float)>& visitor) const
{
    _searching_node_provider->visitAdjacentNodes(_position, visitor);
}

bool GraphSearchingNode::operator<(const GraphSearchingNode& other) const
{
    if(_position.x() != other._position.x())
        return _position.x() < other._position.x();
    if(_position.y() != other._position.y())
        return _position.y() < other._position.y();
    return _position.z() < other._position.z();
}

bool GraphSearchingNode::operator==(const GraphSearchingNode& other) const
{
    return Math::almostEqual(_position.x(), other._position.x()) && Math::almostEqual(_position.y(), other._position.y()) && Math::almostEqual(_position.z(), other._position.z());
}

bool GraphSearchingNode::operator!=(const GraphSearchingNode& other) const
{
    return !(Math::almostEqual(_position.x(), other._position.x()) && Math::almostEqual(_position.y(), other._position.y()) && Math::almostEqual(_position.z(), other._position.z()));
}

}
