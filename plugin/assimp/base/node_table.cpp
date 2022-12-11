#include "assimp/base/node_table.h"

namespace ark {
namespace plugin {
namespace assimp {

bool NodeTable::hasNode(const String& name) const
{
    return _nodes.has(name);
}

AnimationNode& NodeTable::ensureNode(const String& name, const aiMatrix4x4& transform)
{
    const auto iter = _nodes.find(name);
    if(iter != _nodes.end())
        return iter->second;

    uint32_t id = static_cast<uint32_t>(_nodes.size());
    AnimationNode& node = _nodes[name];
    node = AnimationNode(id, transform);
    return node;
}

const Table<String, AnimationNode>& NodeTable::nodes() const
{
    return _nodes;
}

Table<String, AnimationNode>& NodeTable::nodes()
{
    return _nodes;
}

}
}
}
