#include "assimp/base/node.h"

#include <assimp/matrix4x4.inl>

namespace ark {
namespace plugin {
namespace assimp {

Node::Node()
    : _id(0)
{
}

Node::Node(uint32_t id, const aiMatrix4x4& offset)
    : _id(id), _offset(offset)
{
}

}
}
}
