#ifndef ARK_PLUGIN_ASSIMP_BASE_NODE_H_
#define ARK_PLUGIN_ASSIMP_BASE_NODE_H_

#include <assimp/matrix4x4.h>

#include "graphics/base/mat.h"

namespace ark {
namespace plugin {
namespace assimp {

struct Node {
    Node();
    Node(uint32_t id, const aiMatrix4x4& offset);

    uint32_t _id;
    aiMatrix4x4 _offset;
    aiMatrix4x4 _intermediate;

    M4 _final;
};

}
}
}

#endif
