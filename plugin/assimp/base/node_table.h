#ifndef ARK_PLUGIN_ASSIMP_BASE_NODE_TABLE_H_
#define ARK_PLUGIN_ASSIMP_BASE_NODE_TABLE_H_

#include <unordered_map>

#include <assimp/matrix4x4.h>

#include "core/base/string.h"
#include "core/collection/table.h"

#include "graphics/base/mat.h"

#include "assimp/base/node.h"

namespace ark {
namespace plugin {
namespace assimp {

class NodeTable {
public:

    bool hasNode(const String& name) const;
    Node& ensureNode(const String& name, const aiMatrix4x4& transform);

    const Table<String, Node>& nodes() const;
    Table<String, Node>& nodes();

private:
    Table<String, Node> _nodes;
};

}
}
}

#endif
