#include "assimp/base/animation_node.h"

#include <assimp/matrix4x4.inl>

namespace ark {
namespace plugin {
namespace assimp {

AnimationNode::AnimationNode()
    : _id(0)
{
}

AnimationNode::AnimationNode(uint32_t id, const aiMatrix4x4& offset)
    : _id(id), _offset(offset)
{
}

}
}
}
