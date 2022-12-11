#ifndef ARK_PLUGIN_ASSIMP_BASE_NODE_TABLE_H_
#define ARK_PLUGIN_ASSIMP_BASE_NODE_TABLE_H_

#include <assimp/matrix4x4.h>

#include "core/base/string.h"
#include "core/collection/table.h"

#include "graphics/base/mat.h"

#include "assimp/base/animation_node.h"

namespace ark {
namespace plugin {
namespace assimp {

class NodeTable {
public:

    bool hasNode(const String& name) const;
    AnimationNode& ensureNode(const String& name, const aiMatrix4x4& transform);

    const Table<String, AnimationNode>& nodes() const;
    Table<String, AnimationNode>& nodes();

private:
    Table<String, AnimationNode> _nodes;
};

}
}
}

#endif
