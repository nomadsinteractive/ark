#ifndef ARK_PLUGIN_ASSIMP_BASE_NODE_MAP_H_
#define ARK_PLUGIN_ASSIMP_BASE_NODE_MAP_H_

#include <unordered_map>

#include <assimp/matrix4x4.h>

#include "core/base/string.h"

namespace ark {
namespace plugin {
namespace assimp {

class NodeMap {
public:

    struct Node {
        Node();
        Node(uint32_t id, const aiMatrix4x4& transform);

        uint32_t _id;
        aiMatrix4x4 _transform;
    };


    bool hasNode(const String& name) const;
    Node& ensureNode(const String& name, const aiMatrix4x4& transform);

    const std::unordered_map<String, Node>& nodes() const;

private:
    std::unordered_map<String, Node> _nodes;
};

}
}
}

#endif
