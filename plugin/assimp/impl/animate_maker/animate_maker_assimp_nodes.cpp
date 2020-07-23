#include "assimp/impl/animate_maker/animate_maker_assimp_nodes.h"

#include "graphics/base/mat.h"

#include "assimp/util/animate_util.h"

namespace ark {
namespace plugin {
namespace assimp {

AnimateMakerAssimpNodes::AnimateMakerAssimpNodes(sp<Assimp::Importer> importer, const aiAnimation* animation, const aiNode* rootNode, NodeTable boneMapping)
    : _importer(std::move(importer)), _animation(animation), _root_node(rootNode), _bone_mapping(std::move(boneMapping))
{
}

sp<Animate> AnimateMakerAssimpNodes::makeAnimate(const sp<Numeric>& duration)
{
    return sp<AnimateImpl>::make(duration, _animation, _root_node, _bone_mapping);
}

AnimateMakerAssimpNodes::AnimateImpl::AnimateImpl(const sp<Numeric>& duration, const aiAnimation* animation, const aiNode* node, const NodeTable& boneMapping)
    : _ticks_per_sec(static_cast<float>(animation->mTicksPerSecond != 0 ? animation->mTicksPerSecond : 25.0f)), _duration_in_ticks(static_cast<float>(animation->mDuration)),
      _global_inversed_transform(node->mTransformation), _duration(duration), _animation(animation), _root_node(node)
{
    _global_inversed_transform.Inverse();

    for(size_t i = 0; i < boneMapping.nodes().size(); ++i)
        _bone_infos.push_back(sp<BoneInfo>::make());

    for(const auto& i : boneMapping.nodes())
    {
        const Node& node = i.second;
        uint32_t index = node._id;
        _bone_mapping.insert(std::make_pair(i.first, index));
        _bone_infos.at(index)->_offset = node._offset;
    }

    updateHierarchy(_duration->val());
}

bool AnimateMakerAssimpNodes::AnimateImpl::update(uint64_t timestamp)
{
    if(_duration->update(timestamp))
    {
        updateHierarchy(_duration->val());
        return true;
    }
    return false;
}

void AnimateMakerAssimpNodes::AnimateImpl::flat(void* buf)
{
    M4* mat = reinterpret_cast<M4*>(buf);
    for(size_t i = 0; i < _bone_infos.size(); ++i)
        mat[i] = _bone_infos.at(i)->val();
}

uint32_t AnimateMakerAssimpNodes::AnimateImpl::size()
{
    return _bone_infos.size() * sizeof(M4);
}

void AnimateMakerAssimpNodes::AnimateImpl::updateHierarchy(float time)
{
    float animationTime = fmod(time * _ticks_per_sec, _duration_in_ticks);

    AnimateUtil::loadNodeHierarchy(_animation, animationTime, _root_node, aiMatrix4x4(), [this] (const String& nodeName, const aiMatrix4x4& transform) {
        const auto iter = _bone_mapping.find(nodeName);
        if (iter != _bone_mapping.end()) {
            uint32_t boneIndex = iter->second;
            const sp<BoneInfo>& boneInfo = _bone_infos.at(boneIndex);
            boneInfo->_intermediate_transform = transform * boneInfo->_offset;
        }
    });

    for(const sp<BoneInfo>& i : _bone_infos)
    {
        const aiMatrix4x4 finalMatrix = _global_inversed_transform * i->_intermediate_transform;
        i->_final_transform = M4(finalMatrix).transpose();
    }
}

bool AnimateMakerAssimpNodes::BoneInfo::update(uint64_t /*timestamp*/)
{
    return true;
}

M4 AnimateMakerAssimpNodes::BoneInfo::val()
{
    return _final_transform;
}

}
}
}
