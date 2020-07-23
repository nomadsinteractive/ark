#include "assimp/impl/animate_maker/animate_maker_assimp_bones.h"

#include "graphics/base/mat.h"

#include "assimp/util/animate_util.h"

namespace ark {
namespace plugin {
namespace assimp {

AnimateMakerAssimpBones::AnimateMakerAssimpBones(sp<Assimp::Importer> importer, const aiAnimation* animation, const aiNode* rootNode, NodeTable boneMapping)
    : _importer(std::move(importer)), _animation(animation), _root_node(rootNode), _bones(std::move(boneMapping))
{
}

sp<Animate> AnimateMakerAssimpBones::makeAnimate(const sp<Numeric>& duration)
{
    return sp<AnimateImpl>::make(duration, _animation, _root_node, _bones.nodes());
}

AnimateMakerAssimpBones::AnimateImpl::AnimateImpl(const sp<Numeric>& duration, const aiAnimation* animation, const aiNode* node, const Table<String, Node>& bones)
    : _ticks_per_sec(static_cast<float>(animation->mTicksPerSecond != 0 ? animation->mTicksPerSecond : 25.0f)), _duration_in_ticks(static_cast<float>(animation->mDuration)),
      _global_inversed_transform(node->mTransformation), _duration(duration), _animation(animation), _root_node(node), _bones(bones)
{
    _global_inversed_transform.Inverse();
    updateHierarchy(_duration->val());
}

bool AnimateMakerAssimpBones::AnimateImpl::update(uint64_t timestamp)
{
    if(_duration->update(timestamp))
    {
        updateHierarchy(_duration->val());
        return true;
    }
    return false;
}

void AnimateMakerAssimpBones::AnimateImpl::flat(void* buf)
{
    M4* mat = reinterpret_cast<M4*>(buf);
    for(size_t i = 0; i < _bones.size(); ++i)
        mat[i] = _bones.values().at(i)._final;
}

uint32_t AnimateMakerAssimpBones::AnimateImpl::size()
{
    return _bones.size() * sizeof(M4);
}

void AnimateMakerAssimpBones::AnimateImpl::updateHierarchy(float time)
{
    float animationTime = fmod(time * _ticks_per_sec, _duration_in_ticks);

    AnimateUtil::loadNodeHierarchy(_animation, animationTime, _root_node, aiMatrix4x4(), [this] (const String& nodeName, const aiMatrix4x4& transform) {
        const auto iter = _bones.find(nodeName);
        if (iter != _bones.end()) {
            Node& node = iter->second;
            node._intermediate = transform * node._offset;
        }
    });

    for(Node& i : _bones.values())
    {
        const aiMatrix4x4 finalMatrix = _global_inversed_transform * i._intermediate;
        i._final = M4(finalMatrix).transpose();
    }
}

bool AnimateMakerAssimpBones::BoneInfo::update(uint64_t /*timestamp*/)
{
    return true;
}

M4 AnimateMakerAssimpBones::BoneInfo::val()
{
    return _final_transform;
}

}
}
}
