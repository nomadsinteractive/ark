#pragma once

#include <assimp/Importer.hpp>

#include <assimp/anim.h>
#include <assimp/scene.h>

#include "core/base/string.h"
#include "core/collection/table.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/mat.h"

#include "renderer/base/animation_uploader.h"
#include "renderer/inf/animation.h"

#include "assimp/base/node_table.h"

namespace ark::plugin::assimp {

class AnimationAssimpNodes : public Animation {
public:
    typedef std::function<aiMatrix4x4(const AnimationNode& node, const aiMatrix4x4& globalTransformation)> NodeLoaderCallback;

    AnimationAssimpNodes(float tps, const aiAnimation* animation, const aiNode* rootNode, const aiMatrix4x4& globalTransform, Table<String, AnimationNode>& nodes, const NodeLoaderCallback& callback);

    sp<AnimationUploader> makeInput(sp<Numeric> tick) override;
    const std::vector<String>& nodeNames() override;
    std::vector<std::pair<String, sp<Mat4>>> getNodeTranforms(sp<Numeric> duration) override;

private:
    static void loadHierarchy(float tick, const aiNode* node, const aiAnimation* animation, const aiMatrix4x4& parentTransform, Table<String, AnimationNode>& nodes, const NodeLoaderCallback& callback, AnimationFrame& output);
    static void loadNodeHierarchy(float tick, const aiNode* node, const aiAnimation* animation, const aiMatrix4x4& parentTransform, Table<String, AnimationNode>& nodes, const NodeLoaderCallback& callback);

private:
    uint32_t _duration_in_ticks;

    sp<Table<String, uint32_t>> _nodes;
    sp<std::vector<AnimationFrame>> _animation_frames;
};

}
