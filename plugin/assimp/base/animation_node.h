#pragma once

#include <assimp/matrix4x4.h>

#include "graphics/base/mat.h"

namespace ark::plugin::assimp {

struct AnimationNode {
    AnimationNode();
    AnimationNode(uint32_t id, const aiMatrix4x4& offset);

    uint32_t _id;
    aiMatrix4x4 _offset;
    aiMatrix4x4 _intermediate;

    M4 _final;
};

}
