#include "assimp/impl/animation/animation_assimp_nodes.h"

#include "core/inf/variable.h"

#include "graphics/base/mat.h"

namespace ark {
namespace plugin {
namespace assimp {

AnimationAssimpNodes::AnimationAssimpNodes(sp<Assimp::Importer> importer, const aiAnimation* animation, const aiNode* rootNode, const aiMatrix4x4& globalTransform, NodeTable nodes, NodeLoaderCallback callback)
    : _importer(std::move(importer)), _animation(animation), _root_node(rootNode), _ticks_per_sec(animation->mTicksPerSecond != 0 ? static_cast<float>(animation->mTicksPerSecond) : 25.0f), _global_transform(globalTransform),
      _nodes(std::move(nodes)), _callback(std::move(callback))
{
}

sp<Input> AnimationAssimpNodes::makeTransforms(const sp<Numeric>& duration)
{
    return sp<AnimateImpl>::make(duration, _animation, _root_node, _ticks_per_sec, _global_transform, _nodes.nodes(), _callback);
}

float AnimationAssimpNodes::duration()
{
    return static_cast<float>(_animation->mDuration) / _ticks_per_sec;
}

AnimationAssimpNodes::AnimateImpl::AnimateImpl(const sp<Numeric>& duration, const aiAnimation* animation, const aiNode* node, float ticksPerSecond, const aiMatrix4x4& globalTransform, const Table<String, Node>& nodes, NodeLoaderCallback callback)
    : _ticks_per_sec(ticksPerSecond), _duration_in_ticks(static_cast<float>(animation->mDuration)), _global_inversed_transform(node->mTransformation * globalTransform),
      _duration(duration), _animation(animation), _root_node(node), _nodes(nodes), _callback(std::move(callback))
{
    _global_inversed_transform.Inverse();
    updateHierarchy(_duration->val());
}

bool AnimationAssimpNodes::AnimateImpl::update(uint64_t timestamp)
{
    if(_duration->update(timestamp))
    {
        updateHierarchy(_duration->val());
        return true;
    }
    return false;
}

void AnimationAssimpNodes::AnimateImpl::flat(void* buf)
{
    M4* mat = reinterpret_cast<M4*>(buf);
    for(size_t i = 0; i < _nodes.size(); ++i)
        mat[i] = _nodes.values().at(i)._final;
}

uint32_t AnimationAssimpNodes::AnimateImpl::size()
{
    return _nodes.size() * sizeof(M4);
}

void AnimationAssimpNodes::AnimateImpl::updateHierarchy(float time)
{
    float animationTime = fmod(time * _ticks_per_sec, _duration_in_ticks);
    loadNodeHierarchy(animationTime, _root_node, aiMatrix4x4(), _callback);
    for(Node& i : _nodes.values())
    {
        const aiMatrix4x4 finalMatrix = _global_inversed_transform * i._intermediate;
        i._final = M4(finalMatrix).transpose();
    }
}

void AnimationAssimpNodes::AnimateImpl::loadNodeHierarchy(float duration, const aiNode* node, const aiMatrix4x4& parentTransform, const NodeLoaderCallback& callback)
{
    const String nodeName(node->mName.data);
    const aiNodeAnim* pNodeAnim = AnimateUtil::findNodeAnim(_animation, nodeName);

    aiMatrix4x4 nodeTransformation(node->mTransformation);

    if(pNodeAnim)
    {
        aiMatrix4x4 matScale = AnimateUtil::interpolateScale(duration, pNodeAnim);
        aiMatrix4x4 matRotation = AnimateUtil::interpolateRotation(duration, pNodeAnim);
        aiMatrix4x4 matTranslation = AnimateUtil::interpolateTranslation(duration, pNodeAnim);

        nodeTransformation = matTranslation * matRotation * matScale;
    }

    const aiMatrix4x4 globalTransformation = parentTransform * nodeTransformation;
    callback(_nodes, nodeName, globalTransformation);

    for (uint32_t i = 0; i < node->mNumChildren; i++)
        loadNodeHierarchy(duration, node->mChildren[i], globalTransformation, callback);
}

}
}
}
