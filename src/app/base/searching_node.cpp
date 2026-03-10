#include "app/base/searching_node.h"

#include "core/util/math.h"

namespace ark {

const V3& SearchingNode::position() const
{
    return _position;
}

const Optional<float>& SearchingNode::weight() const
{
    return _weight;
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
