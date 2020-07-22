#include "assimp/base/node_map.h"

namespace ark {
namespace plugin {
namespace assimp {

NodeMap::Node::Node()
    : _id(0)
{
}

NodeMap::Node::Node(uint32_t id, const aiMatrix4x4& transform)
    : _id(id), _transform(transform)
{
}

bool NodeMap::hasNode(const String& name) const
{
    return _nodes.find(name) != _nodes.end();
}

NodeMap::Node& NodeMap::ensureNode(const String& name, const aiMatrix4x4& transform)
{
    const auto iter = _nodes.find(name);
    if(iter != _nodes.end())
        return iter->second;

    uint32_t id = static_cast<uint32_t>(_nodes.size());
    Node& node = _nodes[name];
    node = Node(id, transform);
    return node;
}

const std::unordered_map<String, NodeMap::Node>& NodeMap::nodes() const
{
    return _nodes;
}

}
}
}
