#include "assimp/impl/animation/animation_assimp_nodes.h"

#include "core/inf/variable.h"

#include "graphics/base/mat.h"

#include "renderer/base/animation_uploader.h"

#include "util/animate_util.h"

namespace ark::plugin::assimp {

AnimationAssimpNodes::AnimationAssimpNodes(float tps, const aiAnimation* animation, const aiNode* rootNode, const aiMatrix4x4& globalTransform, Table<String, AnimationNode>& nodes, const NodeLoaderCallback& callback)
    : Animation(static_cast<float>(animation->mDuration), tps), _nodes(sp<Table<String, uint32_t>>::make()), _animation_frames(sp<std::vector<AnimationFrame>>::make())
{
    aiMatrix4x4 globalInversedTransform = rootNode->mTransformation * globalTransform;
    globalInversedTransform.Inverse();

    float tpsDefault = animation->mTicksPerSecond != 0 ? static_cast<float>(animation->mTicksPerSecond) : tps;

    _duration_in_ticks = static_cast<uint32_t>(_duration * _tps / tpsDefault);
    float step = _duration / static_cast<float>(_duration_in_ticks);
    for(uint32_t i = 0; i < _duration_in_ticks; ++i)
    {
        AnimationFrame frame;
        loadHierarchy(static_cast<float>(i) * step, rootNode, animation, globalInversedTransform, nodes, callback, frame);
        _animation_frames->push_back(std::move(frame));
    }

     uint32_t index = 0;
     for(const auto& iter : nodes)
         _nodes->push_back(iter.first, index++);
}

std::vector<std::pair<String, sp<Mat4>>> AnimationAssimpNodes::getNodeTranforms(sp<Numeric> duration)
{
    return {};
}

void AnimationAssimpNodes::loadHierarchy(float tick, const aiNode* node, const aiAnimation* animation, const aiMatrix4x4& transform, Table<String, AnimationNode>& nodes, const NodeLoaderCallback& callback, std::vector<M4>& output)
{
    loadNodeHierarchy(tick, node, animation, aiMatrix4x4(), nodes, callback);
    for(AnimationNode& i : nodes.values())
    {
        const aiMatrix4x4 finalMatrix = transform * i._intermediate;
        output.push_back(M4(finalMatrix).transpose());
    }
}

void AnimationAssimpNodes::loadNodeHierarchy(float tick, const aiNode* node, const aiAnimation* animation, const aiMatrix4x4& parentTransform, Table<String, AnimationNode>& nodes, const NodeLoaderCallback& callback)
{
    const String nodeName(node->mName.data);
    const aiNodeAnim* pNodeAnim = AnimateUtil::findNodeAnim(animation, nodeName);

    aiMatrix4x4 nodeTransformation(node->mTransformation);

    if(pNodeAnim)
    {
        aiMatrix4x4 matScale = AnimateUtil::interpolateScale(tick, pNodeAnim);
        aiMatrix4x4 matRotation = AnimateUtil::interpolateRotation(tick, pNodeAnim);
        aiMatrix4x4 matTranslation = AnimateUtil::interpolateTranslation(tick, pNodeAnim);

        nodeTransformation = matTranslation * matRotation * matScale;
    }

    const aiMatrix4x4 globalTransformation = parentTransform * nodeTransformation;

    auto iter = nodes.find(nodeName);
    if (iter != nodes.end())
        iter->second._intermediate = callback(iter->second, globalTransformation);

    for (uint32_t i = 0; i < node->mNumChildren; i++)
        loadNodeHierarchy(tick, node->mChildren[i], animation, globalTransformation, nodes, callback);
}

}
